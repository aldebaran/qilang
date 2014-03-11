/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

/*
%option never-interactive
*/

%{
#include <boost/lexical_cast.hpp>
#include <qilang/node.hpp>
#include <qilang/parser.hpp>
#include "parser_p.hpp"

#define RETURN_OP(a) return a

#define YY_EXTRA_TYPE qilang::Parser*

//allow parsing char from a stream
#define YY_INPUT(buf,result,max_size)   \
{                                       \
   char c;                              \
   (*yyextra->in) >> c;                 \
   if(yyextra->in->eof()) {             \
      result = YY_NULL;                 \
   } else {                             \
      buf[0] = c;                       \
      result = 1;                       \
   }                                    \
}

#include "grammar.tab.hpp"
%}

%option 8bit
%option noyywrap
%option reentrant

/* The next two options tell Flex that we are interfacing with a Bison generated parser;
**   bison-bridge adds an argument yylval to yylex
**   bison-locations adds an argument code yylloc for location tracking.
*/
%option bison-bridge
%option bison-locations

%option yylineno

%option nounistd
%option prefix="qilang_"



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
"subsets"       RETURN_OP(SUBSETS);
"intersects"    RETURN_OP(INTERSECTS);

{FLOAT}           {
  yylval->node = new qilang::FloatNode(boost::lexical_cast<float>(yytext));
  return CONSTANT;
}

{NATURAL}         {
  yylval->node = new qilang::IntNode(boost::lexical_cast<int>(yytext));
  return CONSTANT;
}

{ID}              {
  yylval->node = new qilang::VarNode(yytext);
  return ID;
}

[\']{IDSLASH}[\'] {
   yylval->node = new qilang::VarNode(yytext+1);
   return ID;
}

[\"][^"]*[\"]     {
     yylval->node = new qilang::StringNode(yytext+1);
     return STRING;
}

{BLANKS}          {
}

%%
