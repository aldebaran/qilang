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

%code requires {
  namespace qilang {
    class Parser;
  }
}

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
  EQ                  "="
  GT                  ">"
  GT_EQ               ">="
  LT                  "<"
  LT_EQ               "<="
  NOT_EQ              "!="
  AMPERSAND_AMPERSAND "&&"
  PIPE_PIPE           "||"
  LBRACE              "{"
  RBRACE              "}"
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

  TRUE                "true"
  FALSE               "false"

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
  EMIT                "out"
  PROP                "prop"

  CONST               "const"

  // Core Keywords
  AT                  "at"
  FOR                 "for"
  IF                  "if"


%token <qilang::ConstDataNodePtr>   STRING CONSTANT
%token <std::string>                ID

// the first item here is the last to evaluate, the last item is the first
%left  "||"
%left  "&&"
%left  "==" "!="
%left  "<" "<=" ">" ">="
%left  "|" "&" "^"
%left  "+" "-"
%left  "*" "/" "%"
%precedence "!"
%precedence "["


%%
// #######################################################################################
// # TOP LEVEL
// #######################################################################################

%start toplevel;

%type<qilang::NodePtrVector> toplevel;
toplevel:
  %empty         {}
| toplevel.1 { context->root.insert(context->root.end(), $1.begin(), $1.end()); }

%type<qilang::NodePtrVector> toplevel.1;
toplevel.1:
  toplevel_def            { $$.push_back($1); }
| toplevel.1 toplevel_def { std::swap($$, $1); $$.push_back($2); }

%type<qilang::NodePtr> toplevel_def;
toplevel_def:
  object  { $$ = $1; }
| iface   { $$ = $1; }
| package { $$ = $1; }
| import  { $$ = $1; }
| const   { $$ = $1; }
| struct  { $$ = $1; }
| exp     { $$ = $1; }


// #######################################################################################
// # PACKAGE MANAGEMENT
// #######################################################################################

%type<qilang::NodePtr> package;
package:
  PACKAGE ID                       { $$ = boost::make_shared<qilang::PackageNode>($2);
                                     context->setCurrentPackage($2);
                                   }

%type<qilang::NodePtr> import;
import:
  IMPORT ID                        { $$ = boost::make_shared<qilang::ImportNode>($2); }
| FROM ID IMPORT import_defs       { $$ = boost::make_shared<qilang::ImportNode>($2, $4); }
| FROM ID IMPORT "*"               { qilang::StringVector v;
                                     v.push_back("*");
                                     $$ = boost::make_shared<qilang::ImportNode>($2, v); }

%type<qilang::StringVector> import_defs;
import_defs:
  ID                               { $$.push_back($1); }
| import_defs "," ID               { std::swap($$, $1);
                                     $$.push_back($3);
                                   }

// #######################################################################################
// # OBJECT GRAPH
// #######################################################################################

%type<qilang::StmtNodePtr> object;
object:
  OBJECT type STRING object_defs END { qilang::StringConstDataNode* tnode = static_cast<qilang::StringConstDataNode*>($3.get());
                                       $$ = boost::make_shared<qilang::ObjectDefNode>($2, tnode->value, $4); }

%type<qilang::StmtNodePtrVector> object_defs;
object_defs:
  %empty                       {}
| object_defs.1                { std::swap($$, $1); }

%type<qilang::StmtNodePtrVector> object_defs.1;
object_defs.1:
  object_def                   { $$.push_back($1); }
| object_defs.1 object_def     { std::swap($$, $1); $$.push_back($2); }


%type<qilang::StmtNodePtr> object_def;
object_def:
  object                       { $$ = $1; }
| object_property              { $$ = $1; }
| at_expr                      { $$ = $1; }

%type<qilang::StmtNodePtr> object_property;
object_property:
  ID ":" const_exp             { $$ = boost::make_shared<qilang::PropertyDefNode>($1, $3); }

%type<qilang::StmtNodePtr> at_expr;
at_expr:
  AT ID ":" ID               { $$ = boost::make_shared<qilang::AtNode>($2, $4); }
| AT ID ID END               { $$ = boost::make_shared<qilang::AtNode>($2, $3); }


// #######################################################################################
// # TYPE
// #######################################################################################
%type<qilang::TypeExprNodePtr> type;
type:
  ID                      { $$ = boost::make_shared<qilang::SimpleTypeExprNode>($1); }
| "[" "]" type            { $$ = boost::make_shared<qilang::ListTypeExprNode>($3); }
| "[" type "]" type       { $$ = boost::make_shared<qilang::MapTypeExprNode>($2, $4); }
| "(" tuple_type_defs ")" { $$ = boost::make_shared<qilang::TupleTypeExprNode>($2); }

%type<qilang::TypeExprNodePtrVector> tuple_type_defs;
tuple_type_defs:
  type                      { $$.push_back($1); }
| tuple_type_defs "," type  { std::swap($$, $1); $$.push_back($3); }


// #######################################################################################
// # INTERFACE DECLARATION
// #######################################################################################


%type<qilang::NodePtr> iface;
iface:
  INTERFACE ID "(" inherit_defs ")" interface_defs END { $$ = boost::make_shared<qilang::InterfaceDeclNode>(context->currentPackage(), $2, $4, $6); }
| INTERFACE ID interface_defs END                      { $$ = boost::make_shared<qilang::InterfaceDeclNode>(context->currentPackage(), $2, $3); }

%type<qilang::StringVector> inherit_defs;
inherit_defs:
  %empty         {}
| inherit_defs.1 { std::swap($$, $1); }

%type<qilang::StringVector> inherit_defs.1;
inherit_defs.1:
  ID                     { $$.push_back($1); }
| inherit_defs.1 "," ID  { std::swap($$, $1); $$.push_back($3); }

%type<qilang::DeclNodePtrVector> interface_defs;
interface_defs:
  %empty                       {}
| interface_defs.1             { std::swap($$, $1); }

%type<qilang::DeclNodePtrVector> interface_defs.1;
interface_defs.1:
  interface_def                  { $$.push_back($1); }
| interface_defs.1 interface_def { std::swap($$, $1); $$.push_back($2); }

%type<qilang::DeclNodePtr> interface_def;
interface_def:
  function_decl           { std::swap($$, $1); }
| emit_decl               { std::swap($$, $1); }
| prop_decl               { std::swap($$, $1); }

// fn foooo (t1, t2, t3) tret
%type<qilang::DeclNodePtr> function_decl;
function_decl:
  FN  ID "(" function_args ")" function_arg { $$ = boost::make_shared<qilang::FnDeclNode>($2, $4, $6); }
| FN  ID "(" function_args ")"              { $$ = boost::make_shared<qilang::FnDeclNode>($2, $4); }

%type<qilang::DeclNodePtr> emit_decl;
emit_decl:
  EMIT ID "(" function_args ")"              { $$ = boost::make_shared<qilang::EmitDeclNode>($2, $4); }

%type<qilang::DeclNodePtr> prop_decl;
prop_decl:
  PROP ID "(" function_args ")"             { $$ = boost::make_shared<qilang::PropDeclNode>($2, $4); }


%type<qilang::TypeExprNodePtrVector> function_args;
function_args:
  %empty                          {}
| function_args.1                 { std::swap($$, $1); }

%type<qilang::TypeExprNodePtrVector> function_args.1;
function_args.1:
  function_arg                      { $$.push_back($1); }
| function_args.1 "," function_arg  { std::swap($$, $1);
                                      $$.push_back($3); }

%type<qilang::TypeExprNodePtr> function_arg;
function_arg:
  type    { $$ = $1; }
| ID type { $$ = $2; } //TODO


// #######################################################################################
// # Const
// #######################################################################################

%type<qilang::NodePtr> const;
const:
  CONST ID "=" const_exp    { $$ = boost::make_shared<qilang::ConstDeclNode>(context->currentPackage(), $2, $4); }
| CONST ID type "=" const_exp { $$ = boost::make_shared<qilang::ConstDeclNode>(context->currentPackage(), $2, $3, $5); }


// #######################################################################################
// # Struct
// #######################################################################################
%type<qilang::NodePtr> struct;
struct:
  STRUCT ID struct_field_defs END { $$ = boost::make_shared<qilang::StructDeclNode>(context->currentPackage(), $2, $3); }

%type<qilang::FieldDeclNodePtrVector> struct_field_defs;
struct_field_defs:
  %empty  {}
| struct_field_defs.1 { std::swap($$, $1); }

%type<qilang::FieldDeclNodePtrVector> struct_field_defs.1;
struct_field_defs.1:
  struct_field_def                     { $$.push_back($1); }
| struct_field_defs.1 struct_field_def { std::swap($$, $1); $$.push_back($2); }

%type<qilang::FieldDeclNodePtr> struct_field_def;
struct_field_def:
  ID type   { $$ = boost::make_shared<qilang::FieldDeclNode>($1, $2); }


// #######################################################################################
// # EXPR
// #######################################################################################

//%type<qilang::ExprNodePtr> expr;
//expr:
// exp { std::swap($$, $1); }

%type<qilang::ExprNodePtr> exp;
exp:
  exp "+" exp { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_Plus);}
| exp "-" exp { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_Minus);}
| exp "/" exp { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_Divide);}
| exp "*" exp { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_Multiply);}
| exp "%" exp { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_Modulus);}
| exp "^" exp { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_Xor);}
| exp "|" exp { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_Or);}
| exp "&" exp { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_And);}

exp:
  "!" exp { $$ = boost::make_shared<qilang::UnaryOpExprNode>($2, qilang::UnaryOpCode_Negate);}
| "-" exp { $$ = boost::make_shared<qilang::UnaryOpExprNode>($2, qilang::UnaryOpCode_Minus);}

exp:
  exp "||" exp { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_BoolOr);}
| exp "&&" exp { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_BoolAnd);}

exp:
  const_exp { $$ = boost::make_shared<qilang::ConstDataExprNode>($1); }

exp:
  ID       { $$ = boost::make_shared<qilang::VarExprNode>($1); }

exp:
   exp "==" exp { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_EqEq);}
|  exp "<"  exp { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_Lt);}
|  exp "<=" exp { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_Le);}
|  exp ">"  exp { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_Gt);}
|  exp ">=" exp { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_Ge);}
|  exp "!=" exp { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_Ne);}

exp:
  "(" exp ")" { $$ = $2; }

exp:
  exp "[" exp "]" { $$ = boost::make_shared<qilang::BinaryOpExprNode>($1, $3, qilang::BinaryOpCode_FetchArray);}


// #######################################################################################
// # CONST DATA
// #######################################################################################

%type<qilang::ConstDataNodePtr> const_data;
const_data:
  CONSTANT { $$ = $1; }
| STRING   { $$ = $1; }
| TRUE     { $$ = boost::make_shared<qilang::BoolConstDataNode>(true); }
| FALSE    { $$ = boost::make_shared<qilang::BoolConstDataNode>(false); }
| dict     { $$ = $1; }
| list     { $$ = $1; }
| tuple    { $$ = $1; }

//later it will be more
%type<qilang::ConstDataNodePtr> const_exp;
const_exp:
  const_data { std::swap($$, $1); }


// #######################################################################################
// # CONST DATA: Dict
// #######################################################################################

%type<qilang::ConstDataNodePtr> dict;
dict:
  "{" dict_defs "}" { $$ = boost::make_shared<qilang::DictConstDataNode>($2); }

%type<qilang::ConstDataNodePtrPairVector> dict_defs;
dict_defs:
  %empty      {}
| dict_defs.1 { std::swap($$, $1); }

%type<qilang::ConstDataNodePtrPairVector> dict_defs.1;
dict_defs.1:
  dict_def               { $$.push_back($1); }
| dict_defs "," dict_def { std::swap($$, $1); $$.push_back($3); }


%type<qilang::ConstDataNodePtrPair> dict_def;
dict_def:
  const_exp ":" const_exp { $$ = std::make_pair($1, $3); }


// #######################################################################################
// # CONST DATA: List & Tuple
// #######################################################################################

%type<qilang::ConstDataNodePtr> list;
list:
  "(" list_defs ")" { $$ = boost::make_shared<qilang::ListConstDataNode>($2); }

%type<qilang::ConstDataNodePtr> tuple;
tuple:
  "[" list_defs "]" { $$ = boost::make_shared<qilang::TupleConstDataNode>($2); }

%type<qilang::ConstDataNodePtrVector> list_defs;
list_defs:
  %empty       {}
| list_defs.1  { std::swap($$, $1); }

%type<qilang::ConstDataNodePtrVector> list_defs.1;
list_defs.1:
  const_exp                 { $$.push_back($1); }
| list_defs.1 "," const_exp { std::swap($$, $1); $$.push_back($3); }



%%


void yy::parser::error(const yy::parser::location_type& loc, const std::string& msg)
{
  std::stringstream ss;
  ss << "error: " << loc << ": " << msg << std::endl;
  ss << qilang::getErrorLine(loc);
  throw std::runtime_error(ss.str());
}
