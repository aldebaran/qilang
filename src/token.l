/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

/*
%option never-interactive
*/

%option 8bit
%option noyywrap
%option debug
%option reentrant
%option nounistd
%option prefix="qilang_"
/* by default unmatched scanner rules are print to stdout: disable (instead of disabling ECHO) */
%option nodefault
%option verbose
%option nounput
%option noinput
%option interactive

%{
/* Do not use the C++ generator. The generator only support int yylex()
 * so the FlexLexer.h include file need to be patched, which lead to
 * all sort of complication.
 *
 * Instead: generate a C parser, and compile it in C++.
 *
 * %option c++
 * %option yyclass="qilang::Scanner"
 */

#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <qilang/node.hpp>
#include <qilang/parser.hpp>
#include "parser_p.hpp"
#include "grammar.tab.hpp"

#define RETURN_OP(Symbol)         \
  do { \
    STEP();\
    return yy::parser::make_ ## Symbol(LOC); \
  } while(false)

#define RETURN_VAL(Symbol, Value) \
   do { \
     STEP();\
     return yy::parser::make_ ## Symbol(Value, LOC); \
   } while(false)

#define YY_DECL yy::parser::symbol_type qilang_lex(yyscan_t yyscanner)

#define YY_EXTRA_TYPE qilang::Parser*

#define LOC qilang_get_extra(yyscanner)->loc

// Called via YY_USER_ACTION each time we *advance* the cursor, in
// which case Num is actually the number of bytes read.
#define COLUMNS(Num)                                        \
  LOC.columns(Num)

#define YY_USER_ACTION                          \
  COLUMNS(static_cast<unsigned>(yyleng));

#define LINES(Num)                                  \
  do {                                              \
    LOC.lines(Num);    \
  } while (false)

#define STEP() LOC.step()

#define YY_INPUT(buf, result, max_size) qilang_readsome(yyextra, buf, &result, max_size)

  static void qilang_readsome(qilang::Parser* p, char* buf, size_t *result, size_t max_size)
  {
    if (p->file->in().eof() || p->file->in().fail()) {
      *result = 0;
      return;
    }
    p->file->in().readsome(buf, max_size);
    if (!p->file->in().gcount() && !p->file->in().eof())
    {
      p->file->in().peek();
      p->file->in().readsome(buf, max_size);
    }
    if (p->file->in().bad())
    {
      *result = -1;
      return;
    }
    *result = p->file->in().gcount();
  }

#define yyterminate()                                   \
  return yy::parser::make_END_OF_FILE(LOC)
%}

BLANKS   [ \t]+
EOL      \n|\r|\n\r|\r\n

ID       [a-zA-Z_][_A-Za-z0-9.]*
IDSLASH  [a-zA-Z_/\-][_A-Za-z0-9./\-]*
STRING   [\"][^"]*[\"]
NATURAL  [0-9]+
EXPONENT [eE][-+]?{NATURAL}
UFLOAT   {NATURAL}\.{NATURAL}{EXPONENT}?|{NATURAL}{EXPONENT}
FLOAT    {UFLOAT}

%%

"!"             RETURN_OP(BANG);
"%"             RETURN_OP(PERCENT);
"*"             RETURN_OP(STAR);
"+"             RETURN_OP(PLUS);
"-"             RETURN_OP(MINUS);
"/"             RETURN_OP(SLASH);
"=="            RETURN_OP(EQ_EQ);
"="             RETURN_OP(EQ);
">"             RETURN_OP(GT);
">="            RETURN_OP(GT_EQ);
"<"             RETURN_OP(LT);
"<="            RETURN_OP(LT_EQ);
"!="            RETURN_OP(NOT_EQ);
"&&"|"and"      RETURN_OP(AMPERSAND_AMPERSAND);
"||"|"or"       RETURN_OP(PIPE_PIPE);
"{"             RETURN_OP(LBRACE);
"}"             RETURN_OP(RBRACE);
"("             RETURN_OP(LPAREN);
")"             RETURN_OP(RPAREN);
"~"             RETURN_OP(TILDA);
"["             RETURN_OP(LBRACKET);
"]"             RETURN_OP(RBRACKET);
","             RETURN_OP(COMMA);
"&"             RETURN_OP(AND);
"|"             RETURN_OP(OR);
"^"             RETURN_OP(XOR);
"@"             RETURN_OP(ARO);
":"             RETURN_OP(COLON);

"true"          RETURN_OP(TRUE);
"false"         RETURN_OP(FALSE);

"package"       RETURN_OP(PACKAGE);
"from"          RETURN_OP(FROM);
"import"        RETURN_OP(IMPORT);
"object"        RETURN_OP(OBJECT);
"interface"     RETURN_OP(INTERFACE);
"struct"        RETURN_OP(STRUCT);
"type"          RETURN_OP(TYPE);
"end"           RETURN_OP(END);
"const"         RETURN_OP(CONST);

"fn"            RETURN_OP(FN);
"emit"          RETURN_OP(EMIT);
"prop"          RETURN_OP(PROP);

"at"            RETURN_OP(AT);
"for"           RETURN_OP(FOR);
"if"            RETURN_OP(IF);

{FLOAT}           {
  STEP();
  qilang::LiteralNodePtr node = boost::make_shared<qilang::FloatLiteralNode>(boost::lexical_cast<float>(yytext), qilang::makeLocation(LOC));
  RETURN_VAL(CONSTANT, node);
}

{NATURAL}         {
  STEP();
  qilang::LiteralNodePtr node = boost::make_shared<qilang::IntLiteralNode>(boost::lexical_cast<int>(yytext), qilang::makeLocation(LOC));
  RETURN_VAL(CONSTANT, node);
}

{ID}              {
  STEP();
  RETURN_VAL(ID, std::string(yytext));
}

{STRING}          {   // "   for indentation
  STEP();
  qilang::LiteralNodePtr node = boost::make_shared<qilang::StringLiteralNode>(std::string(yytext + 1, strlen(yytext) - 2), qilang::makeLocation(LOC));
  RETURN_VAL(STRING, node);
}

{BLANKS}          {
STEP();
}

"#".*             {} // One-line comment.

.                 {
                    STEP();
                    std::stringstream ss;
                    ss << "invalid token '" << std::string(yytext) << "'";
                    qilang_get_extra(yyscanner)->parser.error(LOC, ss.str());
                  }
\n                { LINES(1); }

%%
