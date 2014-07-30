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
    const char *builtin[] = { "nothing",
                              "bool" , "char",
                              "int"  , "uint",
                              "int8" , "uint8",
                              "int16", "uint16",
                              "int32", "uint32",
                              "int64", "uint64",
                              "float",
                              "float32", "float64",
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
  // Operators
  BANG                "!"
  PERCENT             "%"
  STAR                "*"
  STARSTAR            "**"
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
  ARROW               "->"

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
  END                 "end"

  TYPEDEF             "typedef"
  ENUM                "enum"

  // IFace Keywords
  FN                  "fn"
  EMIT                "emit"
  PROP                "prop"

  CONST               "const"

  // Core Keywords
  AT                  "at"
  FOR                 "for"
  IF                  "if"


%token <qilang::LiteralNodePtr>   STRING CONSTANT
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
| toplevel.1 { context->_result->ast.insert(context->_result->ast.end(), $1.begin(), $1.end()); }

%type<qilang::NodePtrVector> toplevel.1;
toplevel.1:
  toplevel_def            { $$.push_back($1); }
| toplevel.1 toplevel_def { std::swap($$, $1); $$.push_back($2); }

%type<qilang::NodePtr> toplevel_def;
toplevel_def:
  object        { $$ = $1; }
| iface         { $$ = $1; }
| function_decl { $$ = $1; }
| package       { $$ = $1; }
| import        { $$ = $1; }
| const         { $$ = $1; }
| struct        { $$ = $1; }
| typedef       { $$ = $1; }
| enums         { $$ = $1; }
| exp           { $$ = $1; }


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
                                     $$.push_back($3);
                                   }

// #######################################################################################
// # OBJECT GRAPH
// #######################################################################################

%type<qilang::StmtNodePtr> object;
object:
  OBJECT type STRING object_defs END { qilang::StringLiteralNode* tnode = static_cast<qilang::StringLiteralNode*>($3.get());
                                       $$ = NODE3(ObjectDefNode, @$, $2, tnode->value, $4); }

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
  ID ":" const_exp             { $$ = NODE2(PropertyDefNode, @$, $1, $3); }

%type<qilang::StmtNodePtr> at_expr;
at_expr:
  AT exp ":" ID               { $$ = NODE2(AtNode, @$, $2, $4); }
| AT exp ID END               { $$ = NODE2(AtNode, @$, $2, $3); }


// #######################################################################################
// # TYPE
// #######################################################################################
%type<qilang::TypeExprNodePtr> type;
type:
  ID                      { $$ = makeType(@$, $1); }
| "[" "]" type            { $$ = NODE1(ListTypeExprNode, @$, $3); }
| "[" type "]" type       { $$ = NODE2(MapTypeExprNode, @$, $2, $4); }
| "(" tuple_type_defs ")" { $$ = NODE1(TupleTypeExprNode, @$, $2); }

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
  INTERFACE ID "(" inherit_defs ")" interface_defs END { $$ = NODE3(InterfaceDeclNode, @$, $2, $4, $6); }
| INTERFACE ID interface_defs END                      { $$ = NODE2(InterfaceDeclNode, @$, $2, $3); }

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
  FN  ID "(" param_list ")"              { $$ = NODE2(FnDeclNode, @$, $2, $4); }
| FN  ID "(" param_list ")" "->" type    { $$ = NODE3(FnDeclNode, @$, $2, $4, $7); }

%type<qilang::DeclNodePtr> emit_decl;
emit_decl:
  EMIT ID "(" param_list ")"              { $$ = NODE2(EmitDeclNode, @$, $2, $4); }

%type<qilang::DeclNodePtr> prop_decl;
prop_decl:
  PROP ID "(" param_list ")"             { $$ = NODE2(PropDeclNode, @$, $2, $4); }


%type<qilang::ParamFieldDeclNodePtrVector> param_list;
param_list:
  %empty                          {}
| param_list.1                 { std::swap($$, $1); }

%type<qilang::ParamFieldDeclNodePtrVector> param_list.1;
param_list.1:
  param                       { $$.push_back($1); }
| param_list.1 "," param      { std::swap($$, $1);
                                $$.push_back($3); }
| param_list.1 "," param_end  { std::swap($$, $1);
                                $$.insert($$.end(), $3.begin(), $3.end()); }

%type<qilang::ParamFieldDeclNodePtr> param;
param:
  ID                          { $$ = NODE1(ParamFieldDeclNode, @$, $1); }
| ID type                     { $$ = NODE2(ParamFieldDeclNode, @$, $1, $2); }
| "(" ID "," id_list ")" type { qilang::StringVector sv;
                                sv.push_back($2);
                                sv.insert(sv.end(), $4.begin(), $4.end());
                                $$ = NODE2(ParamFieldDeclNode, @$, sv, $6);
                              }

%type<qilang::ParamFieldDeclNodePtrVector> param_end;
param_end:
  param_vargs                   { $$.push_back($1); }
| param_kwargs                  { $$.push_back($1); }
| param_vargs "," param_kwargs  { $$.push_back($1); $$.push_back($3); }

%type<qilang::ParamFieldDeclNodePtr> param_vargs;
param_vargs:
  "*" ID             { $$ = NODE3(ParamFieldDeclNode, @$, $2, NODE0(VarArgTypeExprNode, @$), qilang::ParamFieldType_VarArgs); }
| "*" ID type        { $$ = NODE3(ParamFieldDeclNode, @$, $2, NODE1(VarArgTypeExprNode, @$, $3), qilang::ParamFieldType_VarArgs); }

%type<qilang::ParamFieldDeclNodePtr> param_kwargs;
param_kwargs:
  "**" ID         { $$ = NODE3(ParamFieldDeclNode, @$, $2, NODE0(KeywordArgTypeExprNode, @$), qilang::ParamFieldType_KeywordArgs); }
| "**" ID type    { $$ = NODE3(ParamFieldDeclNode, @$, $2, NODE1(KeywordArgTypeExprNode, @$, $3), qilang::ParamFieldType_KeywordArgs); }


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
  ID type              { $$.push_back(NODE2(StructFieldDeclNode, @$, $1, $2)); }
// force the ID "," here to avoid reduce conflict between this rules and 'ID type'
| ID "," id_list type  { $$.push_back(NODE2(StructFieldDeclNode, @$, $1, $4));
                         for (unsigned i = 0; i < $3.size(); ++i) {
                            $$.push_back(NODE2(StructFieldDeclNode, @$, $3.at(i), $4));
                         }
                       }
| interface_def        { $$.push_back($1); }


// #######################################################################################
// # EXPR
// #######################################################################################

//%type<qilang::ExprNodePtr> expr;
//expr:
// exp { std::swap($$, $1); }

%type<qilang::ExprNodePtr> exp;
exp:
  exp "+" exp { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_Plus);}
| exp "-" exp { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_Minus);}
| exp "/" exp { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_Divide);}
| exp "*" exp { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_Multiply);}
| exp "%" exp { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_Modulus);}
| exp "^" exp { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_Xor);}
| exp "|" exp { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_Or);}
| exp "&" exp { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_And);}

exp:
  "!" exp { $$ = NODE2(UnaryOpExprNode, @$, $2, qilang::UnaryOpCode_Negate);}
| "-" exp { $$ = NODE2(UnaryOpExprNode, @$, $2, qilang::UnaryOpCode_Minus);}

exp:
  exp "||" exp { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_BoolOr);}
| exp "&&" exp { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_BoolAnd);}

exp:
  const_exp { $$ = NODE1(LiteralExprNode, @$, $1); }

exp:
  ID       { $$ = NODE1(VarExprNode, @$, $1); }

exp:
   exp "==" exp { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_EqEq);}
|  exp "<"  exp { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_Lt);}
|  exp "<=" exp { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_Le);}
|  exp ">"  exp { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_Gt);}
|  exp ">=" exp { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_Ge);}
|  exp "!=" exp { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_Ne);}

exp:
  "(" exp ")" { $$ = $2; }

exp:
  exp "[" exp "]" { $$ = NODE3(BinaryOpExprNode, @$, $1, $3, qilang::BinaryOpCode_FetchArray);}

exp:
  ID "(" ")"          { $$ = NODE1(CallExprNode, @$, $1); }
| ID "(" exp_list ")" { $$ = NODE2(CallExprNode, @$, $1, $3); }

%type<qilang::ExprNodePtrVector> exp_list;
exp_list:
  exp               { $$.push_back($1); }
| exp_list "," exp  { std::swap($$, $1); $$.push_back($3); }


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
