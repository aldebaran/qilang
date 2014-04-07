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
%option yylineno
%option nounistd
%option prefix="qilang_"
/* %option nodefault */
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
   return yy::parser::make_ ## Symbol(yy::parser::location_type())

#define RETURN_VAL(Symbol, Value) \
   return yy::parser::make_ ## Symbol(Value, yy::parser::location_type())

#define YY_DECL yy::parser::symbol_type qilang_lex(yyscan_t yyscanner)

#define YY_EXTRA_TYPE qilang::Parser*


#define ECHO do { ; } while (0)

#define YY_INPUT(buf, result, max_size) qilang_readsome(yyextra, buf, &result, max_size)

  static void qilang_readsome(qilang::Parser* p, char* buf, size_t *result, size_t max_size)
  {
    if (p->in->eof() || p->in->fail()) {
      *result = 0;
      return;
    }
    p->in->readsome(buf, max_size);
    if (!p->in->gcount() && !p->in->eof())
    {
      p->in->peek();
      p->in->readsome(buf, max_size);
    }
    if (p->in->bad())
    {
      *result = -1;
      return;
    }
    *result = p->in->gcount();
  }

#define yyterminate()                                   \
  return yy::parser::make_END_OF_FILE(yy::parser::location_type())
%}

BLANKS   [ \t]+
EOL      \n|\r|\n\r|\r\n

ID       [a-zA-Z_][_A-Za-z0-9.]*
IDSLASH  [a-zA-Z_/\-][_A-Za-z0-9./\-]*

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

">"             RETURN_OP(GT);
">="            RETURN_OP(GT_EQ);
"<"             RETURN_OP(LT);
"<="            RETURN_OP(LT_EQ);
"!="            RETURN_OP(NOT_EQ);
"&&"|"and"      RETURN_OP(AMPERSAND_AMPERSAND);
"||"|"or"       RETURN_OP(PIPE_PIPE);
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

"object"        RETURN_OP(OBJECT);
"interface"     RETURN_OP(INTERFACE);
"struct"        RETURN_OP(STRUCT);
"type"          RETURN_OP(TYPE);
"at"            RETURN_OP(AT);
"end"           RETURN_OP(END);
"for"           RETURN_OP(FOR);
"if"            RETURN_OP(IF);
"fn"            RETURN_OP(FN);
"in"            RETURN_OP(IN);
"out"           RETURN_OP(OUT);
"prop"          RETURN_OP(PROP);

{FLOAT}           {
  qilang::NodePtr node = boost::make_shared<qilang::FloatNode>(boost::lexical_cast<float>(yytext));
  RETURN_VAL(CONSTANT, node);
}

{NATURAL}         {
  qilang::NodePtr node = boost::make_shared<qilang::IntNode>(boost::lexical_cast<int>(yytext));
   RETURN_VAL(CONSTANT, node);
}

{ID}              {
  RETURN_VAL(ID, std::string(yytext));
}

[\']{IDSLASH}[\'] {
  RETURN_VAL(IDSLASH, std::string(yytext));
}

[\"][^"]*[\"]     {   // "   for indentation
  RETURN_VAL(STRING, std::string(yytext));
}

{BLANKS}          {
}

"#".*             {} // One-line comment.

%%
