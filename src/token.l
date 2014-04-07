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

//allow parsing char from a stream
#define YY_INPUT(buf,result,max_size)   \
{                                       \
   char c;                              \
   (*yyextra->in) >> c;                 \
   if(yyextra->in->eof()) {             \
      result = YY_NULL;                 \
   } else {                             \
   std::cout << "char:" << c << std::endl; \
      buf[0] = c;                       \
      result = 1;                       \
   }                                    \
}

#define yyterminate()                                   \
  return yy::parser::make_END_OF_FILE(yy::parser::location_type())

//#define YY_EXTRA_TYPE qilang::Parser*

//#include "grammar.tab.hpp"
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

{FLOAT}           {
  qilang::NodePtr node = boost::make_shared<qilang::FloatNode>(boost::lexical_cast<float>(yytext));
  RETURN_VAL(CONSTANT, node);
}

{NATURAL}         {
  qilang::NodePtr node = boost::make_shared<qilang::IntNode>(boost::lexical_cast<int>(yytext));
  RETURN_VAL(CONSTANT, node);
}

{ID}              {
  qilang::NodePtr node = boost::make_shared<qilang::VarNode>(yytext);
  RETURN_VAL(ID, node);
}

[\']{IDSLASH}[\'] {
  qilang::NodePtr node = boost::make_shared<qilang::VarNode>(yytext+1);
  RETURN_VAL(IDSLASH, node);
}

[\"][^"]*[\"]     {
  qilang::NodePtr node = boost::make_shared<qilang::StringNode>(yytext+1);
  RETURN_VAL(STRING, node);
}

{BLANKS}          {
}

%%
