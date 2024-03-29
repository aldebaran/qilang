/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#ifndef QILANG_NODE_HPP
#define QILANG_NODE_HPP

#include <qilang/api.hpp>

//#include <qilang/node2.hpp>  //import the future
#include <string>
#include <vector>
#include <stdexcept>
#include <qi/types.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <qi/anyvalue.hpp>

namespace qilang {

class Location {
public:
  explicit Location(const std::string& filename)
    : beg_line(0)
    , beg_column(0)
    , end_line(0)
    , end_column(0)
    , filename(filename)
  {}
  Location(int bline = 0, int bcols = 0, int eline = 0, int ecols = 0, const std::string& filename = std::string())
    : beg_line(bline)
    , beg_column(bcols)
    , end_line(eline)
    , end_column(ecols)
    , filename(filename)
  {}

  int beg_line;
  int beg_column;
  int end_line;
  int end_column;
  std::string filename;
};

inline std::ostream& operator<<(std::ostream& os, const Location& loc) {
  if (loc.filename.empty() && loc.beg_line == -1) {
    os << "<noloc>";
    return os;
  }
  if (!loc.filename.empty()) {
    os << loc.filename;
    os << ":";
  }
  os << loc.beg_line << ":" << loc.beg_column;
  return os;
}

class Node;

// STMT: Package Management
class StmtNode;
class PackageNode;
class ImportNode;
class PropertyDefNode;
class VarDefNode;

// EXPR: Const Data
class LiteralNode;  // VIRTUAL
class BoolLiteralNode;
class IntLiteralNode;
class FloatLiteralNode;
class StringLiteralNode;
class ListLiteralNode;
class DictLiteralNode;
class TupleLiteralNode;

// EXPR: Type Expr
class TypeExprNode;        //VIRTUAL
class BuiltinTypeExprNode;
class CustomTypeExprNode;
class VarArgTypeExprNode;
class KeywordArgTypeExprNode;
class ListTypeExprNode;
class MapTypeExprNode;
class TupleTypeExprNode;
class OptionalTypeExprNode;

// EXPR
class ExprNode;        //VIRTUAL: dep on TypeExpr, Literal
class BinaryOpExprNode;
class UnaryOpExprNode;
class VarExprNode;
class LiteralExprNode;
class CallExprNode;

// Interface Declaration
class DeclNode;          //VIRTUAL
class InterfaceDeclNode;
class FnDeclNode;
class ParamFieldDeclNode;
class SigDeclNode;
class PropDeclNode;
class StructDeclNode; //Struct Decl
class StructFieldDeclNode;
class ConstDeclNode;

class TypeDefDeclNode;
class EnumDeclNode;
class EnumFieldDeclNode;

typedef std::vector<std::string>            StringVector;
typedef std::pair<std::string, std::string> StringPair;

typedef boost::shared_ptr<Node>           NodePtr;
typedef std::vector<NodePtr>              NodePtrVector;

typedef boost::shared_ptr<DeclNode>       DeclNodePtr;
typedef std::vector<DeclNodePtr>          DeclNodePtrVector;

typedef boost::shared_ptr<StmtNode>       StmtNodePtr;
typedef std::vector<StmtNodePtr>          StmtNodePtrVector;

typedef boost::shared_ptr<TypeExprNode>   TypeExprNodePtr;
typedef std::vector<TypeExprNodePtr>      TypeExprNodePtrVector;

typedef boost::shared_ptr<ExprNode>       ExprNodePtr;
typedef std::vector<ExprNodePtr>          ExprNodePtrVector;

typedef boost::shared_ptr<LiteralNode>  LiteralNodePtr;
typedef std::vector<LiteralNodePtr>     LiteralNodePtrVector;

typedef std::pair<LiteralNodePtr, LiteralNodePtr> LiteralNodePtrPair;
typedef std::vector<LiteralNodePtrPair>           LiteralNodePtrPairVector;

typedef boost::shared_ptr<ConstDeclNode> ConstDeclNodePtr;

/* All Statements
 */
class NodeVisitor {
protected:
  //to be implemented by the top level visitor
  virtual void doAccept(Node* node) = 0;

public:
  template <typename T>
  void accept(const boost::shared_ptr<T>& nodeptr) {
    doAccept(nodeptr.get());
  }

  //##### #### #####
  //##### DECL
  //##### #### #####
  // Interface Declaration
  virtual void visitDecl(InterfaceDeclNode* node) = 0;
  virtual void visitDecl(FnDeclNode* node) = 0;
  virtual void visitDecl(SigDeclNode* node) = 0;
  virtual void visitDecl(PropDeclNode* node) = 0;
  virtual void visitDecl(ParamFieldDeclNode* node) = 0;

  // Struct Declaration
  virtual void visitDecl(StructDeclNode* node) = 0;
  virtual void visitDecl(ConstDeclNode* node) = 0;
  virtual void visitDecl(StructFieldDeclNode* node) = 0;

  // Typedef
  virtual void visitDecl(TypeDefDeclNode* node) = 0;

  // Enum
  virtual void visitDecl(EnumDeclNode* node) = 0;
  virtual void visitDecl(EnumFieldDeclNode* node) = 0;

  //##### #### #####
  //##### STMT
  //##### #### #####
  // Package Management
  virtual void visitStmt(PackageNode* node) = 0;
  virtual void visitStmt(ImportNode* node) = 0;

  // Object Definitions
  virtual void visitStmt(PropertyDefNode* node) = 0;

  // Definitions
  virtual void visitStmt(VarDefNode* node) = 0;

  //##### #### #####
  //##### Expr
  //##### #### #####
  virtual void visitExpr(BinaryOpExprNode* node) = 0;
  virtual void visitExpr(UnaryOpExprNode* node) = 0;
  virtual void visitExpr(VarExprNode* node) = 0;
  virtual void visitExpr(LiteralExprNode* node) = 0;
  virtual void visitExpr(CallExprNode* node) = 0;

  //##### #### #####
  //##### Data
  //##### #### #####
  virtual void visitData(BoolLiteralNode* node) = 0;
  virtual void visitData(IntLiteralNode* node) = 0;
  virtual void visitData(FloatLiteralNode* node) = 0;
  virtual void visitData(StringLiteralNode* node) = 0;
  virtual void visitData(TupleLiteralNode* node) = 0;
  virtual void visitData(ListLiteralNode* node) = 0;
  virtual void visitData(DictLiteralNode* node) = 0;

  //##### #### #####
  //##### TypeExpr
  //##### #### #####
  virtual void visitTypeExpr(BuiltinTypeExprNode* node) = 0;
  virtual void visitTypeExpr(CustomTypeExprNode* node) = 0;
  virtual void visitTypeExpr(ListTypeExprNode* node) = 0;
  virtual void visitTypeExpr(MapTypeExprNode* node) = 0;
  virtual void visitTypeExpr(TupleTypeExprNode* node) = 0;
  virtual void visitTypeExpr(OptionalTypeExprNode* node) = 0;
  virtual void visitTypeExpr(VarArgTypeExprNode *node) = 0;
  virtual void visitTypeExpr(KeywordArgTypeExprNode *node) = 0;
};


enum NodeKind {
  NodeKind_Expr,
  NodeKind_TypeExpr,
  NodeKind_Literal,
  NodeKind_Decl,
  NodeKind_Stmt
};

enum NodeType {
  NodeType_Package,
  NodeType_Import,

  NodeType_BinOpExpr,
  NodeType_UOpExpr,
  NodeType_VarExpr,
  NodeType_LiteralExpr,

  NodeType_BuiltinTypeExpr,
  NodeType_CustomTypeExpr,
  NodeType_VarArgTypeExpr,
  NodeType_KeywordArgTypeExpr,
  NodeType_MapTypeExpr,
  NodeType_ListTypeExpr,
  NodeType_TupleTypeExpr,
  NodeType_OptionalTypeExpr,

  NodeType_BoolData,
  NodeType_IntData,
  NodeType_FloatData,
  NodeType_StringData,
  NodeType_MapData,
  NodeType_ListData,
  NodeType_TupleData,

  NodeType_PropDef,
  NodeType_VarDef,

  NodeType_InterfaceDecl,
  NodeType_FnDecl,
  NodeType_ParamFieldDecl,
  NodeType_SigDecl,
  NodeType_PropDecl,
  NodeType_TypeDefDecl,
  NodeType_EnumDecl,
  NodeType_EnumFieldDecl,

  NodeType_StructDecl,
  NodeType_StructFieldDecl,
  NodeType_ConstDecl,

  NodeType_Comment,
};

enum TypeKind {
  TypeKind_Interface,
  TypeKind_Enum,
  TypeKind_Struct
};

//Base Node used to describe the AST
class QILANG_API Node
{
public:
  Node(NodeKind kind, NodeType type, const Location& loc, const std::string& comment = "");
  virtual ~Node() {}

  NodeKind kind() const { return _kind; }
  NodeType type() const { return _type; }
  Location loc() const { return _loc; }
  std::string comment() const { return _comment; }

  virtual void accept(NodeVisitor *visitor) = 0;

private:
  NodeKind _kind;
  NodeType _type;
  Location _loc;
  std::string _comment;
};

class KeywordNode : public Node {
public:
  KeywordNode(const Location& loc, const std::string& comment) :
    Node(NodeKind_Literal, NodeType_LiteralExpr, loc, comment)
  {}

  void accept(NodeVisitor*) { throw std::runtime_error("visiting keywords not supported"); };
};

typedef boost::shared_ptr<KeywordNode> KeywordNodePtr;

enum UnaryOpCode {
  UnaryOpCode_Negate,
  UnaryOpCode_Minus
};

enum BinaryOpCode {
  BinaryOpCode_BoolOr,
  BinaryOpCode_BoolAnd,

  BinaryOpCode_And,
  BinaryOpCode_Or,
  BinaryOpCode_Xor,

  BinaryOpCode_EqEq,
  BinaryOpCode_Ne,
  BinaryOpCode_Gt,
  BinaryOpCode_Lt,
  BinaryOpCode_Ge,
  BinaryOpCode_Le,

  BinaryOpCode_Plus,
  BinaryOpCode_Minus,
  BinaryOpCode_Divide,
  BinaryOpCode_Multiply,
  BinaryOpCode_Modulus,

  BinaryOpCode_FetchArray, // a[b]
};

QILANG_API const std::string &UnaryOpCodeToString(UnaryOpCode op);
QILANG_API const std::string &BinaryOpCodeToString(BinaryOpCode op);

// ####################
// # EXPR Node
// ####################

class QILANG_API ExprNode : public Node {
protected:
  explicit ExprNode(NodeKind kind, NodeType type, const Location& loc)
    : Node(kind, type, loc)
  {}
  explicit ExprNode(NodeType type, const Location& loc)
    : Node(NodeKind_Expr, type, loc)
  {}
public:
  virtual void accept(NodeVisitor *visitor) = 0;
};

class QILANG_API BinaryOpExprNode : public ExprNode {
public:
  BinaryOpExprNode(ExprNodePtr left, ExprNodePtr right, BinaryOpCode boc, const Location& loc)
    : ExprNode(NodeType_BinOpExpr, loc)
    , op(boc)
    , left(left)
    , right(right)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitExpr(this); }

  BinaryOpCode op;
  ExprNodePtr  left;
  ExprNodePtr  right;
};

class QILANG_API UnaryOpExprNode : public ExprNode {
public:
  UnaryOpExprNode(ExprNodePtr node, UnaryOpCode op, const Location& loc)
    : ExprNode(NodeType_UOpExpr, loc)
    , op(op)
    , expr(node)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitExpr(this); }

  UnaryOpCode op;
  ExprNodePtr expr;
};

class QILANG_API CallExprNode : public ExprNode {
public:
  CallExprNode(const std::string& id, const Location& loc)
    : ExprNode(NodeType_UOpExpr, loc)
    , name(id)
  {}
  CallExprNode(const std::string& id, const ExprNodePtrVector& args, const Location& loc)
    : ExprNode(NodeType_UOpExpr, loc)
    , name(id)
    , args(args)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitExpr(this); }

  std::string       name;
  ExprNodePtrVector args;
};

class QILANG_API VarExprNode : public ExprNode {
public:
  explicit VarExprNode(const std::string &name, const Location& loc)
    : ExprNode(NodeType_VarExpr, loc)
    , value(name)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitExpr(this); }

  std::string value;
};

class QILANG_API LiteralExprNode : public ExprNode {
public:
  explicit LiteralExprNode(const LiteralNodePtr& data, const Location& loc)
    : ExprNode(NodeType_LiteralExpr, loc)
    , data(data)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitExpr(this); }

  LiteralNodePtr data;
};

// ####################
// # CONST DATA Node
// ####################
class QILANG_API LiteralNode : public Node {
public:
  explicit LiteralNode(NodeType type, const Location& loc)
    : Node(NodeKind_Literal, type, loc)
  {}

  virtual void accept(NodeVisitor* visitor) = 0;
};

class QILANG_API BoolLiteralNode: public LiteralNode {
public:
  explicit BoolLiteralNode(bool val, const Location& loc)
    : LiteralNode(NodeType_BoolData, loc)
    , value(val)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitData(this); }

  bool value;
};

class QILANG_API IntLiteralNode: public LiteralNode {
public:
  explicit IntLiteralNode(qi::uint64_t val, const Location& loc)
    : LiteralNode(NodeType_IntData, loc)
    , value(val)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitData(this); }

  qi::uint64_t value;
};

class QILANG_API FloatLiteralNode: public LiteralNode {
public:
  explicit FloatLiteralNode(double val, const Location& loc)
    : LiteralNode(NodeType_FloatData, loc)
    , value(val)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitData(this); }

  double value;
};

class QILANG_API StringLiteralNode: public LiteralNode {
public:
  explicit StringLiteralNode(const std::string& value, const Location& loc)
    : LiteralNode(NodeType_StringData, loc)
    , value(value)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitData(this); }

  const std::string value;
};

class QILANG_API ListLiteralNode: public LiteralNode {
public:
  explicit ListLiteralNode(const LiteralNodePtrVector& values, const Location& loc)
    : LiteralNode(NodeType_ListData, loc)
    , values(values)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitData(this); }

  LiteralNodePtrVector values;
};

class QILANG_API TupleLiteralNode: public LiteralNode {
public:
  explicit TupleLiteralNode(const LiteralNodePtrVector& values, const Location& loc)
    : LiteralNode(NodeType_TupleData, loc)
    , values(values)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitData(this); }

  LiteralNodePtrVector values;
};

class QILANG_API DictLiteralNode: public LiteralNode {
public:
  explicit DictLiteralNode(const LiteralNodePtrPairVector& values, const Location& loc)
    : LiteralNode(NodeType_MapData, loc)
    , values(values)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitData(this); }

  LiteralNodePtrPairVector values;
};


// ####################
// # TYPE EXPR Node
// ####################
class QILANG_API TypeExprNode : public Node {
public:
  explicit TypeExprNode(NodeType type, const Location& loc)
    : Node(NodeKind_TypeExpr, type, loc)
  {}

  virtual void accept(NodeVisitor* visitor) = 0;
};

// ### WARNING ###
// keep in sync with makeType in grammar.y
enum BuiltinType {
  BuiltinType_Nothing = 0,  //this is void. (useless in qilang itself, but helps with bindings)

  // Basic types
  BuiltinType_Bool,
  BuiltinType_Char,
  BuiltinType_Int,
  BuiltinType_UInt,
  BuiltinType_Int8,
  BuiltinType_UInt8,
  BuiltinType_Int16,
  BuiltinType_UInt16,
  BuiltinType_Int32,
  BuiltinType_UInt32,
  BuiltinType_Int64,
  BuiltinType_UInt64,
  BuiltinType_Float,
  BuiltinType_Float32,
  BuiltinType_Float64,

  BuiltinType_NanoSeconds,
  BuiltinType_MicroSeconds,
  BuiltinType_MilliSeconds,
  BuiltinType_Seconds,
  BuiltinType_Minutes,
  BuiltinType_Hours,
  BuiltinType_QiTimePoint,
  BuiltinType_SteadyTimePoint,
  BuiltinType_SystemTimePoint,

  BuiltinType_String,
  BuiltinType_Raw,
  BuiltinType_Value,
  BuiltinType_Object,
};

class QILANG_API BuiltinTypeExprNode : public TypeExprNode {
public:
  explicit BuiltinTypeExprNode(BuiltinType builtinType, const std::string& sym, const Location& loc)
    : TypeExprNode(NodeType_BuiltinTypeExpr, loc)
    , builtinType(builtinType)
    , value(sym)
  {
    //TODO: check sym is know and match builtinType
  }

  void accept(NodeVisitor* visitor) { visitor->visitTypeExpr(this); }

  BuiltinType builtinType;
  std::string value;
};

class QILANG_API CustomTypeExprNode : public TypeExprNode {
public:
  explicit CustomTypeExprNode(const std::string& sym, const Location& loc)
    : TypeExprNode(NodeType_CustomTypeExpr, loc)
    , value(sym)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitTypeExpr(this); }

  std::string resolved_package;
  std::string resolved_value;
  TypeKind resolved_kind;
  std::string value;
};

class QILANG_API ListTypeExprNode : public TypeExprNode {
public:
  explicit ListTypeExprNode(const TypeExprNodePtr& element, const Location& loc)
    : TypeExprNode(NodeType_ListTypeExpr, loc)
    , element(element)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitTypeExpr(this); }

  TypeExprNodePtr element;
};

class QILANG_API VarArgTypeExprNode : public TypeExprNode {
public:
  explicit VarArgTypeExprNode(const Location& loc)
    : TypeExprNode(NodeType_VarArgTypeExpr, loc)
  {}

  explicit VarArgTypeExprNode(const TypeExprNodePtr& element, const Location& loc)
    : TypeExprNode(NodeType_VarArgTypeExpr, loc)
    , element(element)
  {}

  TypeExprNodePtr effectiveElement() {
    if (element)
      return element;
    return boost::make_shared<BuiltinTypeExprNode>(BuiltinType_Value, "any", loc());
  }

  void accept(NodeVisitor* visitor) { visitor->visitTypeExpr(this); }

  TypeExprNodePtr element;
};

class QILANG_API MapTypeExprNode : public TypeExprNode {
public:
  explicit MapTypeExprNode(const TypeExprNodePtr& key, const TypeExprNodePtr& value, const Location& loc)
    : TypeExprNode(NodeType_MapTypeExpr, loc)
    , key(key)
    , value(value)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitTypeExpr(this); }

  TypeExprNodePtr key;
  TypeExprNodePtr value;
};

class QILANG_API KeywordArgTypeExprNode : public TypeExprNode {
public:
  explicit KeywordArgTypeExprNode(const Location& loc)
    : TypeExprNode(NodeType_KeywordArgTypeExpr, loc)
  {}
  explicit KeywordArgTypeExprNode(const TypeExprNodePtr& value, const Location& loc)
    : TypeExprNode(NodeType_KeywordArgTypeExpr, loc)
    , value(value)
  {}

  TypeExprNodePtr effectiveValue() {
    if (value)
      return value;
    return boost::make_shared<BuiltinTypeExprNode>(BuiltinType_Value, "any", loc());
  }

  void accept(NodeVisitor* visitor) { visitor->visitTypeExpr(this); }

  TypeExprNodePtr value;
};

class QILANG_API TupleTypeExprNode : public TypeExprNode {
public:
  explicit TupleTypeExprNode(const TypeExprNodePtrVector& elements, const Location& loc)
    : TypeExprNode(NodeType_TupleTypeExpr, loc)
    , elements(elements)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitTypeExpr(this); }

  TypeExprNodePtrVector elements;
};

class QILANG_API OptionalTypeExprNode : public TypeExprNode {
public:
  explicit OptionalTypeExprNode(const TypeExprNodePtr& element, const Location& loc)
    : TypeExprNode(NodeType_OptionalTypeExpr, loc)
    , element(element)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitTypeExpr(this); }

  TypeExprNodePtr element;
};

// ####################
// # STMT Node
// ####################
class QILANG_API StmtNode : public Node
{
public:

  StmtNode(NodeType type, const Location& loc)
    : Node(NodeKind_Stmt, type, loc)
  {}

  virtual void accept(NodeVisitor* visitor) = 0;
};

class QILANG_API PackageNode : public StmtNode {
public:
  explicit PackageNode(const std::string& packageName, const Location& loc)
    : StmtNode(NodeType_Package, loc)
    , name(packageName)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitStmt(this); }

public:
  std::string name;
};

enum ImportType {
  ImportType_Package,
  ImportType_List,
  ImportType_All
};

class QILANG_API ImportNode : public StmtNode {
public:
  explicit ImportNode(ImportType importType, const std::string& packageName, const Location& loc)
    : StmtNode(NodeType_Import, loc)
    , name(packageName)
    , importType(importType)
  {}


  ImportNode(ImportType importType, const std::string& packageName, const StringVector& imports, const Location& loc)
    : StmtNode(NodeType_Import, loc)
    , name(packageName)
    , importType(importType)
    , imports(imports)
  {
    if (imports.size() == 0)
      throw std::runtime_error("Empty import list");
  }

  void accept(NodeVisitor* visitor) { visitor->visitStmt(this); }

public:
  std::string  name;
  ImportType   importType;
  StringVector imports;
};



class QILANG_API VarDefNode : public StmtNode {
public:
  VarDefNode(const std::string& name, const TypeExprNodePtr& type, const LiteralNodePtr& data, const Location& loc)
    : StmtNode(NodeType_VarDef, loc)
    , name(name)
    , type(type)
    , data(data)
  {}

  VarDefNode(const std::string& name, const TypeExprNodePtr& type, const Location& loc)
    : StmtNode(NodeType_VarDef, loc)
    , name(name)
    , type(type)
  {}

  TypeExprNodePtr effectiveType() {
    if (type)
      return type;
    return boost::make_shared<BuiltinTypeExprNode>(BuiltinType_Value, "any", loc());
  }

  void accept(NodeVisitor* visitor) { visitor->visitStmt(this); }

  std::string      name;
  TypeExprNodePtr  type;
  LiteralNodePtr data;
};

class QILANG_API PropertyDefNode : public StmtNode {
public:
  PropertyDefNode(const std::string& name, LiteralNodePtr data, const Location& loc)
    : StmtNode(NodeType_PropDef, loc)
    , name(name)
    , data(data)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitStmt(this); }

  std::string      name;
  LiteralNodePtr data;
};

// ####################
// # DECL Node
// ####################

class QILANG_API DeclNode : public Node
{
public:
  DeclNode(NodeType type, const Location& loc, const std::string& comment = "")
    : Node(NodeKind_Decl, type, loc, comment)
  {}

  virtual void accept(NodeVisitor* visitor) = 0;
};

class QILANG_API TypeDefDeclNode : public DeclNode
{
public:
  TypeDefDeclNode(const std::string &name, const TypeExprNodePtr& type, const Location& loc)
    : DeclNode(NodeType_TypeDefDecl, loc)
    , name(name)
    , type(type)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitDecl(this); }

  std::string     name;
  TypeExprNodePtr type;
};

enum EnumFieldType {
  EnumFieldType_Type = 0,
  EnumFieldType_Const = 1
};

class QILANG_API EnumFieldDeclNode : public DeclNode {
public:
  /// @param node Must be a TypeExprNode or a ConstDeclNode
  EnumFieldDeclNode(EnumFieldType fieldType, const NodePtr& node, const Location& loc)
    : DeclNode(NodeType_EnumFieldDecl, loc)
    , fieldType(fieldType)
    , node(node)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitDecl(this); }

  EnumFieldType   fieldType;
  NodePtr         node;
};

typedef boost::shared_ptr<EnumFieldDeclNode> EnumFieldDeclNodePtr;
typedef std::vector<EnumFieldDeclNodePtr>    EnumFieldDeclNodePtrVector;

// a variant... (a list of accepted types)
class QILANG_API EnumDeclNode : public DeclNode
{
public:
  EnumDeclNode(const std::string &name, const EnumFieldDeclNodePtrVector& fields, const Location& loc)
    : DeclNode(NodeType_EnumDecl, loc)
    , name(name)
    , fields(fields)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitDecl(this); }

  std::string                name;
  EnumFieldDeclNodePtrVector fields;
};


class QILANG_API StructFieldDeclNode : public DeclNode {
public:
  StructFieldDeclNode(const std::string &name, const TypeExprNodePtr& type, const Location& loc)
    : StructFieldDeclNode(std::vector<std::string>{name}, type, loc)
  {}

  StructFieldDeclNode(const std::string &name, const TypeExprNodePtr& type, const LiteralNodePtr& data, const Location& loc)
    : StructFieldDeclNode(std::vector<std::string>{name}, type, data, loc)
  {}

  StructFieldDeclNode(const StringVector &names, const TypeExprNodePtr& type, const Location& loc)
    : StructFieldDeclNode(names, type, LiteralNodePtr{}, loc)
  {}

  StructFieldDeclNode(const StringVector &names, const TypeExprNodePtr& type, const LiteralNodePtr& data, const Location& loc)
    : DeclNode(NodeType_StructFieldDecl, loc)
    , names(names)
    , type(type)
    , data(data)
  {}

  TypeExprNodePtr effectiveType() {
    if (type)
      return type;
    return boost::make_shared<BuiltinTypeExprNode>(BuiltinType_Value, "any", loc());
  }

  void accept(NodeVisitor* visitor) { visitor->visitDecl(this); }

  StringVector    names;  //at least one
  TypeExprNodePtr type;
  LiteralNodePtr data;
};
typedef boost::shared_ptr<StructFieldDeclNode> StructFieldDeclNodePtr;
typedef std::vector<StructFieldDeclNodePtr>    StructFieldDeclNodePtrVector;

class QILANG_API StructDeclNode : public DeclNode {
public:
  StructDeclNode(const std::string& name, const DeclNodePtrVector& decls, const Location& loc)
    : DeclNode(NodeType_StructDecl, loc)
    , name(name)
    , decls(decls)
  {}

  StructDeclNode(const std::string& name,
                 const StringVector& inherits,
                 const DeclNodePtrVector& decls,
                 const Location& loc)
    : DeclNode(NodeType_StructDecl, loc)
    , name(name)
    , inherits(inherits)
    , decls(decls)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitDecl(this); }

  std::string       package;
  std::string       name;
  StringVector      inherits;
  DeclNodePtrVector decls;
};

class QILANG_API InterfaceDeclNode : public DeclNode {
public:
  InterfaceDeclNode(const std::string& name, const DeclNodePtrVector& decls, const Location& loc, const std::string& comment = "")
    : DeclNode(NodeType_InterfaceDecl, loc, comment)
    , name(name)
    , values(decls)
  {}

  InterfaceDeclNode(const std::string& name, const StringVector& inherits, const DeclNodePtrVector& decls, const Location& loc, const std::string& comment = "")
    : DeclNode(NodeType_InterfaceDecl, loc, comment)
    , name(name)
    , values(decls)
    , inherits(inherits)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitDecl(this); }

  std::string       name;
  DeclNodePtrVector values;
  StringVector      inherits;
};

enum ParamFieldType {
  ParamFieldType_Normal,
  ParamFieldType_VarArgs,
  ParamFieldType_KeywordArgs,
};

class ParamFieldDeclNode : public DeclNode {
public:
  ParamFieldDeclNode(const std::string &name, const Location& loc)
    : DeclNode(NodeType_ParamFieldDecl, loc)
    , paramType(ParamFieldType_Normal)
  {
    names.push_back(name);
  }
  ParamFieldDeclNode(const std::string &name, const TypeExprNodePtr& type, const Location& loc)
    : DeclNode(NodeType_ParamFieldDecl, loc)
    , type(type)
    , paramType(ParamFieldType_Normal)
  {
    names.push_back(name);
  }
  ParamFieldDeclNode(const std::string &name, ParamFieldType paramType, const Location& loc)
    : DeclNode(NodeType_ParamFieldDecl, loc)
    , paramType(paramType)
  {
    names.push_back(name);
  }
  ParamFieldDeclNode(const std::string &name, const TypeExprNodePtr& type, ParamFieldType paramType, const Location& loc)
    : DeclNode(NodeType_ParamFieldDecl, loc)
    , type(type)
    , paramType(paramType)
  {
    names.push_back(name);
  }

  ParamFieldDeclNode(const StringVector &names, const TypeExprNodePtr& type, const Location& loc)
    : DeclNode(NodeType_ParamFieldDecl, loc)
    , names(names)
    , type(type)
    , paramType(ParamFieldType_Normal)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitDecl(this); }

  bool isVarArgs()     { return paramType == ParamFieldType_VarArgs; }
  bool isKeywordArgs() { return paramType == ParamFieldType_KeywordArgs; }

  TypeExprNodePtr effectiveType() {
    if (type)
      return type;
    return boost::make_shared<BuiltinTypeExprNode>(BuiltinType_Value, "any", loc());
  }
  StringVector    names;
  TypeExprNodePtr type;
  ParamFieldType  paramType;
};
typedef boost::shared_ptr<ParamFieldDeclNode> ParamFieldDeclNodePtr;
typedef std::vector<ParamFieldDeclNodePtr>    ParamFieldDeclNodePtrVector;

class QILANG_API FnDeclNode : public DeclNode {
public:

  FnDeclNode(const std::string& name, const ParamFieldDeclNodePtrVector& args, const TypeExprNodePtr& ret, const Location& loc, const std::string& comment = "")
    : DeclNode(NodeType_FnDecl, loc, comment)
    , name(name)
    , args(args)
    , ret(ret)
  {
    //if ret is nothing, just drop it. (to simplify codegen)
    if (ret && ret->type() == NodeType_BuiltinTypeExpr) {
      BuiltinTypeExprNode* tnode = static_cast<BuiltinTypeExprNode*>(ret.get());
      if (tnode->builtinType == BuiltinType_Nothing)
        this->ret = TypeExprNodePtr();
    }

    unsigned cvar = 0;
    unsigned ckwvar = 0;
    for (unsigned i = 0; i < args.size(); ++i) {
      if (args.at(i)->isVarArgs()) {
        cvar++;
        if (cvar > 1)
          throw std::runtime_error("More than one variable argument specified");
      }
      if (args.at(i)->isKeywordArgs()) {
        ckwvar++;
        if (ckwvar > 1)
          throw std::runtime_error("More than one keyword argument specified");
      }
    }
  }

  FnDeclNode(const std::string& name, const ParamFieldDeclNodePtrVector& args, const Location& loc, const std::string& comment = "")
    : DeclNode(NodeType_FnDecl, loc, comment)
    , name(name)
    , args(args)
  {}

  bool isVariadic() {
    for (unsigned i = 0; i < args.size(); ++i) {
      if (args.at(i)->isVarArgs())
        return true;
    }
    return false;
  }

  bool hasVarArgs();
  bool hasKeywordArgs();

  bool hasNoReturn() {
    if (!ret)
      return true;
    if (ret->type() != NodeType_BuiltinTypeExpr)
      return false;
    BuiltinTypeExprNode* tnode = static_cast<BuiltinTypeExprNode*>(ret.get());
    if (tnode->builtinType == BuiltinType_Nothing)
      return true;
    return false;
  }
  void accept(NodeVisitor* visitor) { visitor->visitDecl(this); }

  TypeExprNodePtr             effectiveRet() {
    if (ret)
      return ret;
    return boost::make_shared<BuiltinTypeExprNode>(BuiltinType_Nothing, "nothing", loc());
  }

public:
  std::string                 name;
  ParamFieldDeclNodePtrVector args;
  TypeExprNodePtr             ret;
};


class QILANG_API SigDeclNode : public DeclNode {
public:
  SigDeclNode(const std::string& name, const ParamFieldDeclNodePtrVector& args, const Location& loc)
    : DeclNode(NodeType_SigDecl, loc)
    , name(name)
    , args(args)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitDecl(this); }

public:
  std::string                 name;
  ParamFieldDeclNodePtrVector args;
};

class QILANG_API PropDeclNode : public DeclNode {
public:
  PropDeclNode(const std::string& name, const ParamFieldDeclNodePtrVector& args, const Location& loc)
    : DeclNode(NodeType_PropDecl, loc)
    , name(name)
    , args(args)
  {}

  void accept(NodeVisitor* visitor) { visitor->visitDecl(this); }

public:
  std::string                 name;
  ParamFieldDeclNodePtrVector args;
};

class QILANG_API ConstDeclNode : public DeclNode {
public:
  ConstDeclNode(const std::string& name, const TypeExprNodePtr& type, const LiteralNodePtr& data, const Location& loc)
    : DeclNode(NodeType_ConstDecl, loc)
    , name(name)
    , type(type)
    , data(data)
  {}

  ConstDeclNode(const std::string& name, const LiteralNodePtr& data, const Location& loc)
    : DeclNode(NodeType_ConstDecl, loc)
    , name(name)
    , data(data)
  {}

  TypeExprNodePtr effectiveType() {
    if (type)
      return type;
    return boost::make_shared<BuiltinTypeExprNode>(BuiltinType_Value, "any", loc());
  }

  void accept(NodeVisitor* visitor) { visitor->visitDecl(this); }

  std::string      name;
  TypeExprNodePtr  type;
  LiteralNodePtr data;
};



}

#endif // NODE_HPP
