/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

%{
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <qilang/node.hpp>
#include <qilang/parser.hpp>
#include "parser_p.hpp"

struct YYLTYPE;
union YYSTYPE;

// stuff from flex that bison needs to know about:
int qilang_lex(YYSTYPE* lvalp, YYLTYPE* llocp, void *scanner);

extern "C" void yyerror(YYLTYPE* locp, qilang::Parser* context, const char* err);

#define scanner context->scanner
%}

%locations
%defines
%error-verbose

//We tell Bison that yyparse should take an extra parameter context
//and that yylex (LanAB_lex) takes an additional argument scanner
%parse-param { qilang::Parser* context }
%lex-param   { void* scanner }

// give us a reentrant parser
%pure-parser
%name-prefix "qilang_"

%token
  BANG                "!"
  PERCENT             "%"
  STAR                "*"
  PLUS                "+"
  MINUS               "-"
  SLASH               "/"
  EQ_EQ               "=="
  GT                  ">"
  GT_EQ               ">="
  LT                  "<"
  LT_EQ               "<="
  NOT_EQ              "!="
  AMPERSAND_AMPERSAND "&&"
  PIPE_PIPE           "||"
  LPAREN               "("
  RPAREN               ")"
  TILDA                "~"
  LBRACKET             "["
  RBRACKET             "]"
  COMMA                ","
  AND                  "&"
  OR                   "|"
  XOR                  "^"
  ARO                  "@"
  SUBSETS    "subsets"
  INTERSECTS "intersects"

%union
{
  qilang::Node* node;
}

%token <node> CONSTANT
%token <node> ID STRING
%token <node> NODE

// the first item here is the last to evaluate, the last item is the first
%left  "||"
%left  "&&"
%left "==" "!="
%left "<" "<=" ">" ">="
%left "|" "&" "^"
%left  "+" "-"
%left  "*" "/" "%"
//%left "subsets" "intersects"
%left  "~" "@"
%right "!"
%left "["

%%

%type<node> exp toplevel;

toplevel:
  exp { context->root = $1;}
;

//comma.opt: /* empty */ | ",";

//exps:
//  /* empty */       { $$ = new Call; }
//| exps.1 comma.opt  { std::swap($$, $1); }
//;

//exps.1:
//  exp             { $$ = new Call; $$->args.push_back($1);}
//| exps.1 "," exp  { std::swap($$, $1); $$->args.push_back($3); }
//;

//call:
//  ID "(" exps ")" { $$ = $3; $$->functionName = $1; free($1);}

//exp:
//  call   { $$ = new CallNode($1->functionName, $1->args); delete $1;}

//exp:
//  "[" exps "]" { $$ = new CallNode("", $2->args); delete $2;}

exp:
  exp "+" exp { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_Plus);}
| exp "-" exp { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_Minus);}
| exp "/" exp { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_Divide);}
| exp "*" exp { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_Multiply);}
| exp "%" exp { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_Modulus);}
| exp "^" exp { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_Xor);}
| exp "|" exp { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_Or);}
| exp "&" exp { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_And);}
//| exp "subsets"    exp { $$ = new BinaryOpNode($1, $3, op_subsets);}
//| exp "intersects" exp { $$ = new BinaryOpNode($1, $3, op_intersects);}

exp:
  "!" exp { $$ = new qilang::UnaryOpNode($2, qilang::UnaryOpCode_Negate);}
| "-" exp { $$ = new qilang::UnaryOpNode($2, qilang::UnaryOpCode_Minus);}

exp:
  exp "||" exp { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_BoolOr);}
| exp "&&" exp { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_BoolAnd);}

exp:
  CONSTANT { $$ = $1; }
| STRING   { $$ = $1; }

exp:
  ID       { $$ = $1; }

exp:
   exp "==" exp { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_EqEq);}
|  exp "<"  exp { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_Lt);}
|  exp "<=" exp { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_Le);}
|  exp ">"  exp { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_Gt);}
|  exp ">=" exp { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_Ge);}
|  exp "!=" exp { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_Ne);}

//exp:
//  exp "~" exp {
//    $$ = new TildaNode($1, $3);
//  }

// The PersistNode has been removed (all keys data are persistent now). But we
//  need to keep this here so we don't get parse errors on existing conditions
exp:
  exp "@" exp { $$ = $1;}

exp:
  "(" exp ")" { $$ = $2;}

exp:
  exp "[" exp "]" { $$ = new qilang::BinaryOpNode($1, $3, qilang::BinaryOpCode_FetchArray);}

%%

void yyerror(YYLTYPE* locp, qilang::Parser* context, const char* err)
{
  std::stringstream ss;

  ss << context->filename << ":" << locp->first_line << ":" << locp->first_column << ": error:" << err;
  throw std::runtime_error(ss.str());
}
