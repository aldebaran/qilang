/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

%{
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <boost/make_shared.hpp>
#include <qilang/node.hpp>
#include <qilang/parser.hpp>
#include "parser_p.hpp"
%}

%locations
%defines
%error-verbose

//use C++ type, instead of old union C type
%define api.value.type variant
// Instead of "yytoken yylex(yylval, yylloc)", use "symbol_type yylex()".
%define api.token.constructor

%language "c++"


//We tell Bison that yyparse should take an extra parameter context
//and that yylex (LanAB_lex) takes an additional argument scanner
%parse-param { qilang::Parser* context }
%lex-param   { qilang::Parser* context }

%code {


  typedef void* yyscan_t;
  yy::parser::symbol_type qilang_lex(yyscan_t lex);

  yy::parser::symbol_type yylex(qilang::Parser* context)
  {
    return qilang_lex(context->scanner);
  }


}


%define api.token.prefix {TOK_}
%token
  //better error reporting
  END_OF_FILE          0 "end of file"
  // Operators
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
  LPAREN              "("
  RPAREN              ")"
  TILDA               "~"
  LBRACKET            "["
  RBRACKET            "]"
  COMMA               ","
  AND                 "&"
  OR                  "|"
  XOR                 "^"
  ARO                 "@"
  COLON               ":"

  // Package Management
  PACKAGE             "package"
  IMPORT              "import"
  FROM                "from"

  // Blocks Types
  OBJECT              "object"
  INTERFACE           "interface"
  STRUCT              "struct"
  TYPE                "type"
  END                 "end"

  // IFace Keywords
  FN                  "fn"
  IN                  "in"
  OUT                 "out"
  PROP                "prop"

  // Core Keywords
  AT                  "at"
  FOR                 "for"
  IF                  "if"

%token <qilang::NodePtr> CONSTANT

%token <std::string> ID IDSLASH STRING

// the first item here is the last to evaluate, the last item is the first
%left  "||"
%left  "&&"
%left  "==" "!="
%left  "<" "<=" ">" ">="
%left  "|" "&" "^"
%left  "+" "-"
%left  "*" "/" "%"
%left  "~" "@"
%right "!"
%left  "["

%%
// #######################################################################################
// # TOP LEVEL
// #######################################################################################

%start toplevel;

%type< std::vector<qilang::NodePtr> > toplevel;
toplevel:
  %empty                {}
| toplevel toplevel_def { context->root.insert(context->root.end(), $1.begin(), $1.end());
                          context->root.push_back($2);
                        }

%type<qilang::NodePtr> toplevel_def;
toplevel_def:
  expr    { $$ = $1; }
| object  { $$ = $1; }
| iface   { $$ = $1; }
| package { $$ = $1; }
| import  { $$ = $1; }

// #######################################################################################
// # PACKAGE MANAGEMENT
// #######################################################################################

%type<qilang::NodePtr> package;
package:
  PACKAGE ID                       { $$ = boost::make_shared<qilang::PackageNode>($2); }

%type<qilang::NodePtr> import;
import:
  IMPORT ID                        { $$ = boost::make_shared<qilang::ImportNode>($2); }
| FROM ID IMPORT import_defs       { $$ = boost::make_shared<qilang::ImportNode>($2, $4); }
| FROM ID IMPORT "*"               { std::vector<std::string> v; v.push_back("*");
                                     $$ = boost::make_shared<qilang::ImportNode>($2, v); }

%type< std::vector<std::string> > import_defs;
import_defs:
  ID                               { $$.push_back($1); }
| import_defs "," ID               { std::swap($$, $1);
                                     $$.push_back($3);
                                   }

// #######################################################################################
// # OBJECT GRAPH
// #######################################################################################

%type<qilang::NodePtr> object;
object:
  OBJECT ID STRING object_defs END { $$ = boost::make_shared<qilang::ObjectNode>($2, $3, $4); }

%type< std::vector<qilang::NodePtr> > object_defs;
object_defs:
  %empty                       {}
| object_def                   { $$.push_back($1); }
| object_defs object_def       { std::swap($$, $1);
                                 $$.push_back($2);
                               }

%type<qilang::NodePtr> object_def;
object_def:
  object                       { $$ = $1; }
| object_property              { $$ = $1; }
| at_expr                      { $$ = $1; }

%type<qilang::NodePtr> object_property;
object_property:
  ID ":" exp                   { $$ = boost::make_shared<qilang::PropertyNode>($1, $3); }

%type<qilang::NodePtr> at_expr;
at_expr:
  AT ID ":" ID               { $$ = boost::make_shared<qilang::AtNode>($2, $4); }
| AT ID ID END               { $$ = boost::make_shared<qilang::AtNode>($2, $3); }


// #######################################################################################
// # INTERFACE DECLARATION
// #######################################################################################

%type<qilang::NodePtr> iface;
iface:
  INTERFACE ID interface_defs END { $$ = boost::make_shared<qilang::InterfaceDeclNode>($2, $3); }

%type< std::vector<qilang::NodePtr> > interface_defs;
interface_defs:
  %empty                       {}
| interface_def                { $$.push_back($1); }
| interface_defs interface_def { std::swap($$, $1); $$.push_back($2); }

%type <qilang::NodePtr> interface_def;
interface_def:
  function_decl           { std::swap($$, $1); }
| in_decl                 { std::swap($$, $1); }
| out_decl                { std::swap($$, $1); }
| prop_decl               { std::swap($$, $1); }

// fn foooo (t1, t2, t3) tret
%type<qilang::NodePtr> function_decl;
function_decl:
  FN  ID "(" function_args ")" function_arg { $$ = boost::make_shared<qilang::FnDeclNode>($2, $4, $6); }

%type<qilang::NodePtr> in_decl;
in_decl:
  IN  ID "(" function_args ")"              { $$ = boost::make_shared<qilang::InDeclNode>($2, $4); }

%type<qilang::NodePtr> out_decl;
out_decl:
  OUT ID "(" function_args ")"              { $$ = boost::make_shared<qilang::OutDeclNode>($2, $4); }

%type<qilang::NodePtr> prop_decl;
prop_decl:
  PROP ID "(" function_args ")"             { $$ = boost::make_shared<qilang::PropDeclNode>($2, $4); }


%type< std::vector<std::string> > function_args;
function_args:
  %empty                          {}
| function_arg                    { $$.push_back($1); }
| function_args "," function_arg  { std::swap($$, $1);
                                    $$.push_back($3); }

%type<std::string> function_arg;
function_arg:
  ID { $$ = $1; }


// #######################################################################################
// # EXPR
// #######################################################################################

%type <qilang::NodePtr> expr;
expr:
 exp { $$ = boost::make_shared<qilang::ExprNode>($1); }

%type<qilang::NodePtr> exp;
exp:
  exp "+" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Plus);}
| exp "-" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Minus);}
| exp "/" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Divide);}
| exp "*" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Multiply);}
| exp "%" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Modulus);}
| exp "^" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Xor);}
| exp "|" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Or);}
| exp "&" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_And);}

exp:
  "!" exp { $$ = boost::make_shared<qilang::UnaryOpNode>($2, qilang::UnaryOpCode_Negate);}
| "-" exp { $$ = boost::make_shared<qilang::UnaryOpNode>($2, qilang::UnaryOpCode_Minus);}

exp:
  exp "||" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_BoolOr);}
| exp "&&" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_BoolAnd);}

exp:
  CONSTANT { $$ = $1; }
| STRING   { $$ = boost::make_shared<qilang::StringConstNode>($1); }

exp:
  ID       { $$ = boost::make_shared<qilang::VarNode>($1); }

exp:
   exp "==" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_EqEq);}
|  exp "<"  exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Lt);}
|  exp "<=" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Le);}
|  exp ">"  exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Gt);}
|  exp ">=" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Ge);}
|  exp "!=" exp { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_Ne);}

// The PersistNode has been removed (all keys data are persistent now). But we
//  need to keep this here so we don't get parse errors on existing conditions
exp:
  exp "@" exp { $$ = $1;}

exp:
  "(" exp ")" { $$ = $2;}

exp:
exp "[" exp "]" { $$ = boost::make_shared<qilang::BinaryOpNode>($1, $3, qilang::BinaryOpCode_FetchArray);}

%%

void yy::parser::error(const yy::parser::location_type& loc, const std::string& msg)
{
  std::stringstream ss;
  ss << "error: " << loc << ": " << msg << std::endl;
  ss << qilang::getErrorLine(loc);
  throw std::runtime_error(ss.str());
}
