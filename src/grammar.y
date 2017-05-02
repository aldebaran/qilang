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

  #define NODE0(TYPE, LOC) \
    boost::make_shared< qilang::TYPE >(qilang::makeLocation(LOC))
  #define NODE1(TYPE, LOC, a) \
    boost::make_shared< qilang::TYPE >(a, qilang::makeLocation(LOC))
  #define NODE2(TYPE, LOC, a, b) \
    boost::make_shared< qilang::TYPE >(a, b, qilang::makeLocation(LOC))
  #define NODE3(TYPE, LOC, a, b, c) \
    boost::make_shared< qilang::TYPE >(a, b, c, qilang::makeLocation(LOC))
  #define NODE4(TYPE, LOC, a, b, c, d) \
    boost::make_shared< qilang::TYPE >(a, b, c, d, qilang::makeLocation(LOC))

  #define NODEC0(TYPE, LOC, N) \
    boost::make_shared< qilang::TYPE >(qilang::makeLocation(LOC), N->comment())
  #define NODEC1(TYPE, LOC, N, a) \
    boost::make_shared< qilang::TYPE >(a, qilang::makeLocation(LOC), N->comment())
  #define NODEC2(TYPE, LOC, N, a, b) \
    boost::make_shared< qilang::TYPE >(a, b, qilang::makeLocation(LOC), N->comment())
  #define NODEC3(TYPE, LOC, N, a, b, c) \
    boost::make_shared< qilang::TYPE >(a, b, c, qilang::makeLocation(LOC), N->comment())
  #define NODEC4(TYPE, LOC, N, a, b, c, d) \
    boost::make_shared< qilang::TYPE >(a, b, c, d, qilang::makeLocation(LOC), N->comment())
}

%code {

  typedef void* yyscan_t;
  yy::parser::symbol_type qilang_lex(yyscan_t lex);

  yy::parser::symbol_type yylex(qilang::Parser* context)
  {
    return qilang_lex(context->scanner);
  }

  qilang::TypeExprNodePtr makeType(const yy::location& loc, const std::string& id) {
    // ### WARNING ###
    // keep in sync with node.hpp enum BuiltinType
    const char *builtin[] = {
      "nothing", "bool" , "char",
      "int"  , "uint",
      "int8" , "uint8",
      "int16", "uint16",
      "int32", "uint32",
      "int64", "uint64",
      "float",
      "float32", "float64",
      "nsec", "usec", "msec", "sec", "min", "hour",
      "qitimepoint", "steadytimepoint", "systemtimepoint",
      "str", "any", "obj", 0 };
    int index = 0;
    const char *t = builtin[index];
    while (t != 0) {
      if (id == t)
        return NODE2(BuiltinTypeExprNode, loc, static_cast<qilang::BuiltinType>(index), id);
      index++;
      t = builtin[index];
    }
    return NODE1(CustomTypeExprNode, loc, id);
  }

}

%define api.token.prefix {TOK_}
%token
  //better error reporting
  END_OF_FILE          0 "end of file"

  EQ                  "="
  GT                  ">"
  LT                  "<"
  LPAREN              "("
  RPAREN              ")"
  COMMA               ","
  COLON               ":"
  ARROW               "->"

  TRUE                "true"
  FALSE               "false"

  // Package Management
  PACKAGE             "package"
  IMPORT              "import"
  FROM                "from"

  // Blocks Types
  STRUCT              "struct"
  END                 "end"

  TYPEDEF             "typedef"
  ENUM                "enum"

  // IFace Keywords
  SIG                 "sig"
  PROP                "prop"
  CONST               "const"

  // Basic Types Keywords
  VEC                 "Vec"
  MAP                 "Map"
  TUPLE               "Tuple"

%token <qilang::KeywordNodePtr>
  INTERFACE           "interface"
  FN                  "fn"

%token <qilang::LiteralNodePtr>   STRING CONSTANT
%token <std::string>              ID

%%
// #######################################################################################
// # TOP LEVEL
// #######################################################################################

%start toplevel;

%type<qilang::NodePtrVector> toplevel;
toplevel:
  %empty         {}
| toplevel.1 { context->_result->ast.insert(context->_result->ast.end(), $1.begin(), $1.end()); }

%type<qilang::NodePtrVector> toplevel.1;
toplevel.1:
  toplevel_def            { $$.push_back($1); }
| toplevel.1 toplevel_def { std::swap($$, $1); $$.push_back($2); }

%type<qilang::NodePtr> toplevel_def;
toplevel_def:
  iface         { $$ = $1; }
| function_decl { $$ = $1; }
| package       { $$ = $1; }
| import        { $$ = $1; }
| const         { $$ = $1; }
| struct        { $$ = $1; }
| typedef       { $$ = $1; }
| enums         { $$ = $1; }


%type<qilang::StringVector> id_list;
id_list:
  ID             { $$.push_back($1); }
| id_list "," ID { std::swap($$, $1); $$.push_back($3); }

// #######################################################################################
// # PACKAGE MANAGEMENT
// #######################################################################################

%type<qilang::NodePtr> package;
package:
  PACKAGE ID                       { $$ = NODE1(PackageNode, @$, $2); }

%type<qilang::NodePtr> import;
import:
  IMPORT ID                        { $$ = NODE2(ImportNode, @$, qilang::ImportType_Package, $2); }
| FROM ID IMPORT "*"               { $$ = NODE2(ImportNode, @$, qilang::ImportType_All, $2); }
| FROM ID IMPORT import_defs       { $$ = NODE3(ImportNode, @$, qilang::ImportType_List, $2, $4); }

%type<qilang::StringVector> import_defs;
import_defs:
  ID                               { $$.push_back($1); }
| import_defs "," ID               { std::swap($$, $1);
                                     $$.push_back($3); }

// #######################################################################################
// # TYPE
// #######################################################################################
%type<qilang::TypeExprNodePtr> type;
type:
  ID                                { $$ = makeType(@$, $1); }
| "Vec" "<" type ">"                { $$ = NODE1(ListTypeExprNode, @$, $3); }
| "Map" "<" type "," type ">"       { $$ = NODE2(MapTypeExprNode, @$, $3, $5); }
| "Tuple" "<" tuple_type_defs ">"   { $$ = NODE1(TupleTypeExprNode, @$, $3); }

%type<qilang::TypeExprNodePtrVector> tuple_type_defs;
tuple_type_defs:
  type                      { $$.push_back($1); }
| tuple_type_defs "," type  { std::swap($$, $1); $$.push_back($3); }


// #######################################################################################
// # TYPEDEF
// #######################################################################################

%type<qilang::DeclNodePtr> typedef;
typedef:
  TYPEDEF type ID           { $$ = NODE2(TypeDefDeclNode, @$, $3, $2); }



// #######################################################################################
// # ENUM
// #######################################################################################

%type<qilang::DeclNodePtr> enums;
enums:
  ENUM ID enums_defs END    { $$ = NODE2(EnumDeclNode, @$, $2, $3); }

%type<qilang::EnumFieldDeclNodePtrVector> enums_defs;
enums_defs:
  %empty       {}
| enums_defs.1 { std::swap($$, $1); }

%type<qilang::EnumFieldDeclNodePtrVector> enums_defs.1;
enums_defs.1:
  enums_def               { $$.push_back($1); }
| enums_defs.1 enums_def  { std::swap($$, $1); $$.push_back($2); }

%type<qilang::EnumFieldDeclNodePtr> enums_def;
enums_def:
  const  { $$ = NODE2(EnumFieldDeclNode, @$, qilang::EnumFieldType_Const, $1); }
| type   { $$ = NODE2(EnumFieldDeclNode, @$, qilang::EnumFieldType_Type, $1); }



// #######################################################################################
// # INTERFACE DECLARATION
// #######################################################################################


%type<qilang::NodePtr> iface;
iface:
  INTERFACE ID "(" inherit_defs ")" interface_defs END { $$ = NODEC3(InterfaceDeclNode, @$, $1, $2, $4, $6); }
| INTERFACE ID interface_defs END                      { $$ = NODEC2(InterfaceDeclNode, @$, $1, $2, $3); }

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
| sig_decl                { std::swap($$, $1); }
| prop_decl               { std::swap($$, $1); }

// fn foooo (t1, t2, t3) tret
%type<qilang::DeclNodePtr> function_decl;
function_decl:
  FN  ID "(" param_list ")"              { $$ = NODEC2(FnDeclNode, @$, $1, $2, $4); }
| FN  ID "(" param_list ")" "->" type    { $$ = NODEC3(FnDeclNode, @$, $1, $2, $4, $7); }

%type<qilang::DeclNodePtr> sig_decl;
sig_decl:
  SIG ID "(" param_list ")"              { $$ = NODE2(SigDeclNode, @$, $2, $4); }

%type<qilang::DeclNodePtr> prop_decl;
prop_decl:
  PROP ID "(" param_list ")"             { $$ = NODE2(PropDeclNode, @$, $2, $4); }


%type<qilang::ParamFieldDeclNodePtrVector> param_list;
param_list:
  %empty                          {}
| param_list.1                 { std::swap($$, $1); }

%type<qilang::ParamFieldDeclNodePtrVector> param_list.1;
param_list.1:
  param_list.2                { std::swap($$, $1); }
| param_list.2 "," param_end  { std::swap($$, $1);
                                $$.insert($$.end(), $3.begin(), $3.end()); }

%type<qilang::ParamFieldDeclNodePtrVector> param_list.2;
param_list.2:
  param                       { $$.push_back($1); }
| param_list.2 "," param      { std::swap($$, $1);
                                $$.push_back($3); }

%type<qilang::ParamFieldDeclNodePtr> param;
param:
  ID ":" type                 { $$ = NODE2(ParamFieldDeclNode, @$, $1, $3); }

%type<qilang::ParamFieldDeclNodePtrVector> param_end;
param_end:
  param_vargs                   { $$.push_back($1); }
| param_kwargs                  { $$.push_back($1); }
| param_vargs "," param_kwargs  { $$.push_back($1); $$.push_back($3); }

%type<qilang::ParamFieldDeclNodePtr> param_vargs;
param_vargs:
  "*" ID            { $$ = NODE3(ParamFieldDeclNode, @$, $2, NODE0(VarArgTypeExprNode, @$), qilang::ParamFieldType_VarArgs); }
| "*" ID ":" type   { $$ = NODE3(ParamFieldDeclNode, @$, $2, NODE1(VarArgTypeExprNode, @$, $4), qilang::ParamFieldType_VarArgs); }

%type<qilang::ParamFieldDeclNodePtr> param_kwargs;
param_kwargs:
  "**" ID           { $$ = NODE3(ParamFieldDeclNode, @$, $2, NODE0(KeywordArgTypeExprNode, @$), qilang::ParamFieldType_KeywordArgs); }
| "**" ID ":" type  { $$ = NODE3(ParamFieldDeclNode, @$, $2, NODE1(KeywordArgTypeExprNode, @$, $4), qilang::ParamFieldType_KeywordArgs); }


// #######################################################################################
// # Const
// #######################################################################################

%type<qilang::NodePtr> const;
const:
  CONST ID "=" const_exp    { $$ = NODE2(ConstDeclNode, @$, $2, $4); }
| CONST ID type "=" const_exp { $$ = NODE3(ConstDeclNode, @$, $2, $3, $5); }


// #######################################################################################
// # Struct
// #######################################################################################
%type<qilang::NodePtr> struct;
struct:
  STRUCT ID struct_field_defs END                      { $$ = NODE2(StructDeclNode, @$, $2, $3); }
| STRUCT ID "(" inherit_defs ")" struct_field_defs END { $$ = NODE3(StructDeclNode, @$, $2, $4, $6); }

%type<qilang::DeclNodePtrVector> struct_field_defs;
struct_field_defs:
  %empty  {}
| struct_field_defs.1 { std::swap($$, $1); }

%type<qilang::DeclNodePtrVector> struct_field_defs.1;
struct_field_defs.1:
  struct_field_def                     { std::swap($$, $1); }
| struct_field_defs.1 struct_field_def { std::swap($$, $1); $$.insert($$.end(), $2.begin(), $2.end()); }

%type<qilang::DeclNodePtrVector> struct_field_def;
struct_field_def:
  ID ":" type              { $$.push_back(NODE2(StructFieldDeclNode, @$, $1, $3)); }
| ID ":" type "=" const_exp { $$.push_back(NODE3(StructFieldDeclNode, @$, $1, $3, $5)); }
| ID "," id_list ":" type  { $$.push_back(NODE2(StructFieldDeclNode, @$, $1, $5));
                         for (unsigned i = 0; i < $3.size(); ++i) {
                            $$.push_back(NODE2(StructFieldDeclNode, @$, $3.at(i), $5));
                         }
                       }
| interface_def        { $$.push_back($1); }

// #######################################################################################
// # CONST DATA
// #######################################################################################

%type<qilang::LiteralNodePtr> const_data;
const_data:
  CONSTANT { $$ = $1; }
| STRING   { $$ = $1; }
| TRUE     { $$ = NODE1(BoolLiteralNode, @$, true); }
| FALSE    { $$ = NODE1(BoolLiteralNode, @$, false); }
| dict     { $$ = $1; }
| list     { $$ = $1; }
| tuple    { $$ = $1; }

//later it will be more
%type<qilang::LiteralNodePtr> const_exp;
const_exp:
  const_data { std::swap($$, $1); }


// #######################################################################################
// # CONST DATA: Dict
// #######################################################################################

%type<qilang::LiteralNodePtr> dict;
dict:
  "{" dict_defs "}" { $$ = NODE1(DictLiteralNode, @$, $2); }

%type<qilang::LiteralNodePtrPairVector> dict_defs;
dict_defs:
  %empty      {}
| dict_defs.1 { std::swap($$, $1); }

%type<qilang::LiteralNodePtrPairVector> dict_defs.1;
dict_defs.1:
  dict_def               { $$.push_back($1); }
| dict_defs "," dict_def { std::swap($$, $1); $$.push_back($3); }


%type<qilang::LiteralNodePtrPair> dict_def;
dict_def:
  const_exp ":" const_exp { $$ = std::make_pair($1, $3); }


// #######################################################################################
// # CONST DATA: List & Tuple
// #######################################################################################

%type<qilang::LiteralNodePtr> list;
list:
  "[" list_defs "]" { $$ = NODE1(ListLiteralNode, @$, $2); }

%type<qilang::LiteralNodePtr> tuple;
tuple:
  "(" ")"           { qilang::LiteralNodePtrVector empty;
                      $$ = NODE1(TupleLiteralNode, @$, empty);
                    }
//1 elt tuple conflict with (exp), force the ","
| "(" const_exp "," list_defs ")" { $4.insert($4.begin(), $2);
                                    $$ = NODE1(TupleLiteralNode, @$, $4);
                                  }

%type<qilang::LiteralNodePtrVector> list_defs;
list_defs:
  %empty       {}
| list_defs.1  { std::swap($$, $1); }

%type<qilang::LiteralNodePtrVector> list_defs.1;
list_defs.1:
  const_exp                 { $$.push_back($1); }
| list_defs.1 "," const_exp { std::swap($$, $1); $$.push_back($3); }



%%

void yy::parser::error(const yy::parser::location_type& loc, const std::string& msg)
{
  throw qilang::ParseException(qilang::makeLocation(loc), msg);
}
