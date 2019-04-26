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
    yy::parser::symbol_type tok = yy::parser::make_ ## Symbol(LOC); \
    STEP(); \
    return tok; \
  } while(false)

#define RETURN_OP2(Symbol)         \
  do { \
    qilang::KeywordNodePtr node = boost::make_shared<qilang::KeywordNode>(qilang::makeLocation(LOC), qilang_get_extra(yyscanner)->lastComment); \
    yy::parser::symbol_type tok = yy::parser::make_ ## Symbol(node, LOC); \
    STEP(); \
    return tok; \
  } while(false)

#define RETURN_VAL(Symbol, Value) \
   do { \
     yy::parser::symbol_type tok = yy::parser::make_ ## Symbol(Value, LOC); \
     STEP(); \
     return tok; \
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

#ifdef _WIN32
  static bool isatty(int fp) {
    return false;
  }
#endif

// yy_n_chars has changed in 2.5.36 from int to yy_size_t to fix issue 1961902
// https://github.com/westes/flex/commit/9ba3187a537d6a58d345f2874d06087fd4050399
// Then it changed back to int in 2.6.1 to fix https://github.com/westes/flex/issues/53
// https://github.com/westes/flex/commit/4a8eccf830e36f6611dc9cc6debd815ff32dcaf5
//
// Apple ships its own build of flex with Mac OS.
// It is based on upstream flex 2.5.35 but it does include
// the 1961902 change, and thus behaves like the upstream 2.5.36 version
// (see https://opensource.apple.com/source/flex/flex-26/patches/W64-32.diff.auto.html).
//
// Hence, whenever flex has version 2.5.35 and we are on Mac OS, we assume this is the
// apple-provided flex build, and we do as if it was the upstream flex 2.5.36.
#if ((defined(__APPLE__) && YY_FLEX_MAJOR_VERSION == 2 && YY_FLEX_MINOR_VERSION == 5 && YY_FLEX_SUBMINOR_VERSION == 35) \
    || (YY_FLEX_MAJOR_VERSION == 2 && YY_FLEX_MINOR_VERSION == 5 && YY_FLEX_SUBMINOR_VERSION >= 36) \
    || (YY_FLEX_MAJOR_VERSION == 2 && YY_FLEX_MINOR_VERSION == 6 && YY_FLEX_SUBMINOR_VERSION < 1))
  static void qilang_readsome(qilang::Parser* p, char* buf, yy_size_t *result, size_t max_size)
#else
  static void qilang_readsome(qilang::Parser* p, char* buf, int *result, size_t max_size)
#endif
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

"="             RETURN_OP(EQ);
">"             RETURN_OP(GT);
"<"             RETURN_OP(LT);
"("             RETURN_OP(LPAREN);
")"             RETURN_OP(RPAREN);
","             RETURN_OP(COMMA);
":"             RETURN_OP(COLON);
"->"            RETURN_OP(ARROW);

"true"          RETURN_OP(TRUE);
"false"         RETURN_OP(FALSE);

"package"       RETURN_OP(PACKAGE);
"from"          RETURN_OP(FROM);
"import"        RETURN_OP(IMPORT);
"struct"        RETURN_OP(STRUCT);
"end"           RETURN_OP(END);
"const"         RETURN_OP(CONST);

"fn"            RETURN_OP2(FN);
"sig"           RETURN_OP(SIG);
"prop"          RETURN_OP(PROP);

"typedef"       RETURN_OP(TYPEDEF);
"enum"          RETURN_OP(ENUM);

"interface"     RETURN_OP2(INTERFACE);

"Vec"           RETURN_OP(VEC);
"Map"           RETURN_OP(MAP);
"Tuple"         RETURN_OP(TUPLE);
"Opt"           RETURN_OP(OPT);

{FLOAT}           {
  qilang::LiteralNodePtr node = boost::make_shared<qilang::FloatLiteralNode>(boost::lexical_cast<float>(yytext), qilang::makeLocation(LOC));
  RETURN_VAL(CONSTANT, node);
}

{NATURAL}         {
  qilang::LiteralNodePtr node = boost::make_shared<qilang::IntLiteralNode>(boost::lexical_cast<int>(yytext), qilang::makeLocation(LOC));
  RETURN_VAL(CONSTANT, node);
}

{ID}              {
  RETURN_VAL(ID, std::string(yytext));
}

{STRING}          {   // "   for indentation
  qilang::LiteralNodePtr node = boost::make_shared<qilang::StringLiteralNode>(std::string(yytext + 1, strlen(yytext) - 2), qilang::makeLocation(LOC));
  RETURN_VAL(STRING, node);
}

{BLANKS}          {
  STEP();
}

"#!".*            {
  qilang::Parser* parser = qilang_get_extra(yyscanner);
  parser->lastComment += (yytext+2);
  parser->lastComment += '\n';
  parser->linesSinceLastComment = 0;
}

"#".*             { STEP(); } // One-line comment.

"//!".*           {
  qilang::Parser* parser = qilang_get_extra(yyscanner);
  parser->lastComment += (yytext+3);
  parser->lastComment += '\n';
  parser->linesSinceLastComment = 0;
}

"//".*            { STEP(); } // One-line comment.

.                 {
                    STEP();
                    std::stringstream ss;
                    ss << "invalid token '" << std::string(yytext) << "'";
                    qilang_get_extra(yyscanner)->parser.error(LOC, ss.str());
                  }

{EOL}             {
  qilang::Parser* parser = qilang_get_extra(yyscanner);
  if (!parser->lastComment.empty())
  {
    if (++parser->linesSinceLastComment == 2)
    {
      parser->linesSinceLastComment = 0;
      parser->lastComment.clear();
    }
  }
  LINES(1);
  STEP();
}

%%
