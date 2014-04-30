/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#ifndef QILANG_NODE_HPP
#define QILANG_NODE_HPP

#include <qilang/api.hpp>

#include <string>
#include <vector>
#include <stdexcept>
#include <qi/types.hpp>
#include <boost/shared_ptr.hpp>
#include <qitype/anyvalue.hpp>

namespace qilang {

class Location {
public:
  explicit Location(const std::string& filename)
    : beg_line(-1)
    , beg_column(0)
    , end_line(-1)
    , end_column(0)
    , filename(filename)
  {}
  Location(int bline = -1, int bcols = 0, int eline = -1, int ecols = 0, const std::string& filename = std::string())
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
    if (loc.beg_line != -1)
      os << ":";
  }
  if (loc.beg_line != -1)
    os << loc.beg_line << ":" << loc.beg_column;
  return os;
}

class Node;

// STMT: Package Management
class StmtNode;
class PackageNode;
class ImportNode;
class ObjectDefNode;
class PropertyDefNode;
class AtNode;
class VarDefNode;
class CommentNode;

// EXPR: Const Data
class ConstDataNode;  // VIRTUAL
class BoolConstDataNode;
class IntConstDataNode;
class FloatConstDataNode;
class StringConstDataNode;
class ListConstDataNode;
class DictConstDataNode;
class TupleConstDataNode;

// EXPR: Type Expr
class TypeExprNode;        //VIRTUAL
class BuiltinTypeExprNode;
class CustomTypeExprNode;
class ListTypeExprNode;
class MapTypeExprNode;
class TupleTypeExprNode;

// EXPR
class ExprNode;        //VIRTUAL: dep on TypeExpr, ConstData
class BinaryOpExprNode;
class UnaryOpExprNode;
class VarExprNode;
class ConstDataExprNode;

// Interface Declaration
class DeclNode;          //VIRTUAL
class InterfaceDeclNode;
class FnDeclNode;
class EmitDeclNode;
class PropDeclNode;
class StructDeclNode; //Struct Decl
class FieldDeclNode;
class ConstDeclNode;


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

typedef boost::shared_ptr<ConstDataNode>  ConstDataNodePtr;
typedef std::vector<ConstDataNodePtr>     ConstDataNodePtrVector;

typedef std::pair<ConstDataNodePtr, ConstDataNodePtr> ConstDataNodePtrPair;
typedef std::vector<ConstDataNodePtrPair>             ConstDataNodePtrPairVector;


/* All Statements
 */
class DeclNodeVisitor {
public:
  virtual void acceptDecl(const DeclNodePtr& node) = 0;

  // Interface Declaration
  virtual void visitDecl(InterfaceDeclNode* node) = 0;
  virtual void visitDecl(FnDeclNode* node) = 0;
  virtual void visitDecl(EmitDeclNode* node) = 0;
  virtual void visitDecl(PropDeclNode* node) = 0;

  // Struct Declaration
  virtual void visitDecl(StructDeclNode* node) = 0;
  virtual void visitDecl(ConstDeclNode* node) = 0;
  virtual void visitDecl(FieldDeclNode* node) = 0;
};

class StmtNodeVisitor {
public:
  virtual void acceptStmt(const StmtNodePtr& node) = 0;

  // Package Management
  virtual void visitStmt(PackageNode* node) = 0;
  virtual void visitStmt(ImportNode* node) = 0;

  // Object Definitions
  virtual void visitStmt(ObjectDefNode* node) = 0;
  virtual void visitStmt(PropertyDefNode* node) = 0;
  virtual void visitStmt(AtNode* node) = 0;

  // Definitions
  virtual void visitStmt(VarDefNode* node) = 0;

  virtual void visitStmt(CommentNode* node) = 0;
};


class ExprNodeVisitor {
public:
  virtual void acceptExpr(const ExprNodePtr& node) = 0;

  // Expr
  virtual void visitExpr(BinaryOpExprNode* node) = 0;
  virtual void visitExpr(UnaryOpExprNode* node) = 0;
  virtual void visitExpr(VarExprNode* node) = 0;
  virtual void visitExpr(ConstDataExprNode* node) = 0;
};

/** Const Data Expression Visitor
 */
class ConstDataNodeVisitor {
public:
  virtual void acceptData(const ConstDataNodePtr& node) = 0;

  virtual void visitData(BoolConstDataNode* node) = 0;
  virtual void visitData(IntConstDataNode* node) = 0;
  virtual void visitData(FloatConstDataNode* node) = 0;
  virtual void visitData(StringConstDataNode* node) = 0;
  virtual void visitData(TupleConstDataNode* node) = 0;
  virtual void visitData(ListConstDataNode* node) = 0;
  virtual void visitData(DictConstDataNode* node) = 0;
};

/** Type Expression Visitor
 */
class TypeExprNodeVisitor {
protected:
public:
  virtual void acceptTypeExpr(const TypeExprNodePtr& node) = 0;

  virtual void visitTypeExpr(BuiltinTypeExprNode* node) = 0;
  virtual void visitTypeExpr(CustomTypeExprNode* node) = 0;
  virtual void visitTypeExpr(ListTypeExprNode* node) = 0;
  virtual void visitTypeExpr(MapTypeExprNode* node) = 0;
  virtual void visitTypeExpr(TupleTypeExprNode* node) = 0;
};


enum NodeKind {
  NodeKind_Expr,
  NodeKind_TypeExpr,
  NodeKind_ConstData,
  NodeKind_Decl,
  NodeKind_Stmt
};

enum NodeType {
  NodeType_Package,
  NodeType_Import,

  NodeType_BinOpExpr,
  NodeType_UOpExpr,
  NodeType_VarExpr,
  NodeType_ConstDataExpr,

  NodeType_BuiltinTypeExpr,
  NodeType_CustomTypeExpr,
  NodeType_MapTypeExpr,
  NodeType_ListTypeExpr,
  NodeType_TupleTypeExpr,

  NodeType_BoolData,
  NodeType_IntData,
  NodeType_FloatData,
  NodeType_StringData,
  NodeType_MapData,
  NodeType_ListData,
  NodeType_TupleData,

  NodeType_ObjectDef,
  NodeType_PropDef,
  NodeType_VarDef,
  NodeType_At,

  NodeType_InterfaceDecl,
  NodeType_FnDecl,
  NodeType_EmitDecl,
  NodeType_PropDecl,

  NodeType_StructDecl,
  NodeType_FieldDecl,
  NodeType_ConstDecl,

  NodeType_Comment,
};

//Base Node used to describe the AST
class QILANG_API Node
{
public:
  Node(NodeKind kind, NodeType type, const Location& loc);
  virtual ~Node() {}

  NodeKind kind() const { return _kind; }
  NodeType type() const { return _type; }
  Location loc() const { return _loc; }

private:
  NodeKind _kind;
  NodeType _type;
  Location _loc;
};



#if 0
class Node2;
typedef std::map<std::string, qi::AnyValue> AttributeMap;
typedef boost::shared_ptr<Node2> Node2Ptr;
typedef std::vector<Node2Ptr> NodeVector;

class QILANG_API Node2 {
public:
  explicit Node2(NodeType type, const AttributeMap& map, const NodeVector& children);
  explicit Node2(NodeType type, const AttributeMap& map);
  explicit Node2(NodeType type);

  Node2& setAttr(const std::string& name, qi::AutoAnyReference value);
  qi::AnyValue attr(const std::string& name);

  Node2& addChild(const Node2& node);

  NodeVector children() { return _children; }

  NodeKind     _kind;
  NodeType     _type;
  AttributeMap _attributes;
  NodeVector   _children;
};

class DictConstDataNode2: public Node2 {
  DictConstDataNode2(ConstDataNodePtrPairVector datas):
    Node2(NodeKind_ConstData)
  {
    setAttr("data", datas);
  }
};
#endif



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
  virtual void accept(ExprNodeVisitor *visitor) = 0;
};

class QILANG_API BinaryOpExprNode : public ExprNode {
public:
  BinaryOpExprNode(ExprNodePtr n1, ExprNodePtr n2, BinaryOpCode boc, const Location& loc)
    : ExprNode(NodeType_BinOpExpr, loc)
    , op(boc)
    , n1(n1)
    , n2(n2)
  {}

  void accept(ExprNodeVisitor* visitor) { visitor->visitExpr(this); }

  BinaryOpCode op;
  ExprNodePtr  n1;
  ExprNodePtr  n2;
};

class QILANG_API  UnaryOpExprNode : public ExprNode {
public:
  UnaryOpExprNode(ExprNodePtr node, UnaryOpCode op, const Location& loc)
    : ExprNode(NodeType_UOpExpr, loc)
    , op(op)
    , n1(node)
  {}

  void accept(ExprNodeVisitor* visitor) { visitor->visitExpr(this); }

  UnaryOpCode op;
  ExprNodePtr n1;

};

class QILANG_API VarExprNode : public ExprNode {
public:
  explicit VarExprNode(const std::string &name, const Location& loc)
    : ExprNode(NodeType_VarExpr, loc)
    , value(name)
  {}

  void accept(ExprNodeVisitor* visitor) { visitor->visitExpr(this); }

  std::string value;
};

class QILANG_API ConstDataExprNode : public ExprNode {
public:
  explicit ConstDataExprNode(const ConstDataNodePtr& data, const Location& loc)
    : ExprNode(NodeType_ConstDataExpr, loc)
    , data(data)
  {}

  void accept(ExprNodeVisitor* visitor) { visitor->visitExpr(this); }

  ConstDataNodePtr data;
};

// ####################
// # CONST DATA Node
// ####################
class QILANG_API ConstDataNode : public Node {
public:
  explicit ConstDataNode(NodeType type, const Location& loc)
    : Node(NodeKind_ConstData, type, loc)
  {}

  virtual void accept(ConstDataNodeVisitor* visitor) = 0;
};

class QILANG_API BoolConstDataNode: public ConstDataNode {
public:
  explicit BoolConstDataNode(bool val, const Location& loc)
    : ConstDataNode(NodeType_BoolData, loc)
    , value(val)
  {}

  void accept(ConstDataNodeVisitor* visitor) { visitor->visitData(this); }

  bool value;
};

class QILANG_API IntConstDataNode: public ConstDataNode {
public:
  explicit IntConstDataNode(qi::uint64_t val, const Location& loc)
    : ConstDataNode(NodeType_IntData, loc)
    , value(val)
  {}

  void accept(ConstDataNodeVisitor* visitor) { visitor->visitData(this); }

  qi::uint64_t value;
};

class QILANG_API FloatConstDataNode: public ConstDataNode {
public:
  explicit FloatConstDataNode(double val, const Location& loc)
    : ConstDataNode(NodeType_FloatData, loc)
    , value(val)
  {}

  void accept(ConstDataNodeVisitor* visitor) { visitor->visitData(this); }

  double value;
};

class QILANG_API StringConstDataNode: public ConstDataNode {
public:
  explicit StringConstDataNode(const std::string& value, const Location& loc)
    : ConstDataNode(NodeType_StringData, loc)
    , value(value)
  {}

  void accept(ConstDataNodeVisitor* visitor) { visitor->visitData(this); }

  const std::string value;
};

class QILANG_API ListConstDataNode: public ConstDataNode {
public:
  explicit ListConstDataNode(const ConstDataNodePtrVector& values, const Location& loc)
    : ConstDataNode(NodeType_ListData, loc)
    , values(values)
  {}

  void accept(ConstDataNodeVisitor* visitor) { visitor->visitData(this); }

  ConstDataNodePtrVector values;
};

class QILANG_API TupleConstDataNode: public ConstDataNode {
public:
  explicit TupleConstDataNode(const ConstDataNodePtrVector& values, const Location& loc)
    : ConstDataNode(NodeType_TupleData, loc)
    , values(values)
  {}

  void accept(ConstDataNodeVisitor* visitor) { visitor->visitData(this); }

  ConstDataNodePtrVector values;
};

class QILANG_API DictConstDataNode: public ConstDataNode {
public:
  explicit DictConstDataNode(const ConstDataNodePtrPairVector& values, const Location& loc)
    : ConstDataNode(NodeType_MapData, loc)
    , values(values)
  {}

  void accept(ConstDataNodeVisitor* visitor) { visitor->visitData(this); }

  ConstDataNodePtrPairVector values;
};


// ####################
// # TYPE EXPR Node
// ####################
class QILANG_API TypeExprNode : public Node {
public:
  explicit TypeExprNode(NodeType type, const Location& loc)
    : Node(NodeKind_TypeExpr, type, loc)
  {}

  virtual void accept(TypeExprNodeVisitor* visitor) = 0;
};

//warning keep in sync with makeType in grammar.y
enum BuiltinType {
  BuiltinType_Bool = 0,
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
  BuiltinType_String,
  BuiltinType_Value,
  BuiltinType_Object,
};

class QILANG_API BuiltinTypeExprNode : public TypeExprNode {
public:
  explicit BuiltinTypeExprNode(BuiltinType builtinType, const std::string& sym, const Location& loc)
    : TypeExprNode(NodeType_BuiltinTypeExpr, loc)
    , builtinType(builtinType)
    , value(sym)
  {}

  void accept(TypeExprNodeVisitor* visitor) { visitor->visitTypeExpr(this); }

  BuiltinType builtinType;
  std::string value;
};

class QILANG_API CustomTypeExprNode : public TypeExprNode {
public:
  explicit CustomTypeExprNode(const std::string& sym, const Location& loc)
    : TypeExprNode(NodeType_CustomTypeExpr, loc)
    , value(sym)
  {}

  void accept(TypeExprNodeVisitor* visitor) { visitor->visitTypeExpr(this); }

  std::string resolved_package;
  std::string resolved_value;
  std::string value;
};

class QILANG_API ListTypeExprNode : public TypeExprNode {
public:
  explicit ListTypeExprNode(const TypeExprNodePtr& element, const Location& loc)
    : TypeExprNode(NodeType_ListTypeExpr, loc)
    , element(element)
  {}

  void accept(TypeExprNodeVisitor* visitor) { visitor->visitTypeExpr(this); }

  TypeExprNodePtr element;
};

class QILANG_API MapTypeExprNode : public TypeExprNode {
public:
  explicit MapTypeExprNode(const TypeExprNodePtr& key, const TypeExprNodePtr& value, const Location& loc)
    : TypeExprNode(NodeType_MapTypeExpr, loc)
    , key(key)
    , value(value)
  {}

  void accept(TypeExprNodeVisitor* visitor) { visitor->visitTypeExpr(this); }

  TypeExprNodePtr key;
  TypeExprNodePtr value;
};

class QILANG_API TupleTypeExprNode : public TypeExprNode {
public:
  explicit TupleTypeExprNode(const TypeExprNodePtrVector& elements, const Location& loc)
    : TypeExprNode(NodeType_TupleTypeExpr, loc)
    , elements(elements)
  {}

  void accept(TypeExprNodeVisitor* visitor) { visitor->visitTypeExpr(this); }

  TypeExprNodePtrVector elements;
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

  virtual void accept(StmtNodeVisitor* visitor) = 0;
};

class QILANG_API PackageNode : public StmtNode {
public:
  explicit PackageNode(const std::string& packageName, const Location& loc)
    : StmtNode(NodeType_Package, loc)
    , name(packageName)
  {}

  void accept(StmtNodeVisitor* visitor) { visitor->visitStmt(this); }

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

  void accept(StmtNodeVisitor* visitor) { visitor->visitStmt(this); }

public:
  std::string  name;
  ImportType   importType;
  StringVector imports;
};



class QILANG_API VarDefNode : public StmtNode {
public:
  VarDefNode(const std::string& name, const TypeExprNodePtr& type, const ConstDataNodePtr& data, const Location& loc)
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

  void accept(StmtNodeVisitor* visitor) { visitor->visitStmt(this); }

  std::string      name;
  TypeExprNodePtr  type;
  ConstDataNodePtr data;
};

// Object Motion.MoveTo "titi"
class QILANG_API ObjectDefNode : public StmtNode {
public:
  ObjectDefNode(const TypeExprNodePtr& type, const std::string& name, const StmtNodePtrVector& defs, const Location& loc)
    : StmtNode(NodeType_ObjectDef, loc)
    , type(type)
    , name(name)
    , values(defs)
  {}

  void accept(StmtNodeVisitor* visitor) { visitor->visitStmt(this); }

  TypeExprNodePtr   type;
  std::string       name;
  StmtNodePtrVector values;
};

// myprop: tititoto
class QILANG_API PropertyDefNode : public StmtNode {
public:
  PropertyDefNode(const std::string& name, ConstDataNodePtr data, const Location& loc)
    : StmtNode(NodeType_PropDef, loc)
    , name(name)
    , data(data)
  {}

  void accept(StmtNodeVisitor* visitor) { visitor->visitStmt(this); }

  std::string      name;
  ConstDataNodePtr data;
};

class QILANG_API AtNode : public StmtNode {
public:
  AtNode(const std::string& sender, const std::string& receiver, const Location& loc)
    : StmtNode(NodeType_At, loc)
    , sender(sender)
    , receiver(receiver)
  {}

  void accept(StmtNodeVisitor* visitor) { visitor->visitStmt(this); }

public:
  std::string sender;
  std::string receiver;
};

class QILANG_API CommentNode : public StmtNode {
public:
  CommentNode(const std::string& comments, const Location& loc)
    : StmtNode(NodeType_Comment, loc)
    , comments(comments)
  {}


  void accept(StmtNodeVisitor* visitor) { visitor->visitStmt(this); }

  std::string      comments;
};

// ####################
// # DECL Node
// ####################

class QILANG_API DeclNode : public Node
{
public:
  DeclNode(NodeType type, const Location& loc)
    : Node(NodeKind_Decl, type, loc)
  {}

  virtual void accept(DeclNodeVisitor* visitor) = 0;
};

class QILANG_API FieldDeclNode : public DeclNode {
public:
  FieldDeclNode(const std::string &name, const TypeExprNodePtr& type, const Location& loc)
    : DeclNode(NodeType_FieldDecl, loc)
    , name(name)
    , type(type)
  {}

  void accept(DeclNodeVisitor* visitor) { visitor->visitDecl(this); }

  std::string     name;
  TypeExprNodePtr type;
};
typedef boost::shared_ptr<FieldDeclNode> FieldDeclNodePtr;
typedef std::vector<FieldDeclNodePtr>    FieldDeclNodePtrVector;

class QILANG_API StructDeclNode : public DeclNode {
public:
  StructDeclNode(const std::string& name, const FieldDeclNodePtrVector& vardefs, const Location& loc)
    : DeclNode(NodeType_StructDecl, loc)
    , name(name)
    , fields(vardefs)
  {}

  void accept(DeclNodeVisitor* visitor) { visitor->visitDecl(this); }

  std::string            package;
  std::string            name;
  FieldDeclNodePtrVector fields;
};

// Object Motion.MoveTo "titi"
class QILANG_API InterfaceDeclNode : public DeclNode {
public:
  InterfaceDeclNode(const std::string& name, const DeclNodePtrVector& decls, const Location& loc)
    : DeclNode(NodeType_InterfaceDecl, loc)
    , name(name)
    , values(decls)
  {}

  InterfaceDeclNode(const std::string& name, const StringVector& inherits, const DeclNodePtrVector& decls, const Location& loc)
    : DeclNode(NodeType_InterfaceDecl, loc)
    , name(name)
    , values(decls)
    , inherits(inherits)
  {}

  void accept(DeclNodeVisitor* visitor) { visitor->visitDecl(this); }

  //std::string       package;
  std::string       name;
  DeclNodePtrVector values;
  StringVector      inherits;
};

class QILANG_API FnDeclNode : public DeclNode {
public:
  FnDeclNode(const std::string& name, const TypeExprNodePtrVector& args, const TypeExprNodePtr& ret, const Location& loc)
    : DeclNode(NodeType_FnDecl, loc)
    , name(name)
    , args(args)
    , ret(ret)
  {}

  FnDeclNode(const std::string& name, const TypeExprNodePtrVector& args, const Location& loc)
    : DeclNode(NodeType_FnDecl, loc)
    , name(name)
    , args(args)
  {}

  void accept(DeclNodeVisitor* visitor) { visitor->visitDecl(this); }

public:
  std::string             name;
  TypeExprNodePtrVector   args;
  TypeExprNodePtr         ret;
};


class QILANG_API EmitDeclNode : public DeclNode {
public:
  EmitDeclNode(const std::string& name, const TypeExprNodePtrVector& args, const Location& loc)
    : DeclNode(NodeType_EmitDecl, loc)
    , name(name)
    , args(args)
  {}

  void accept(DeclNodeVisitor* visitor) { visitor->visitDecl(this); }

public:
  std::string           name;
  TypeExprNodePtrVector args;
};

class QILANG_API PropDeclNode : public DeclNode {
public:
  PropDeclNode(const std::string& name, const TypeExprNodePtrVector& args, const Location& loc)
    : DeclNode(NodeType_PropDecl, loc)
    , name(name)
    , args(args)
  {}

  void accept(DeclNodeVisitor* visitor) { visitor->visitDecl(this); }

public:
  std::string           name;
  TypeExprNodePtrVector args;
};

class QILANG_API ConstDeclNode : public DeclNode {
public:
  ConstDeclNode(const std::string& name, const TypeExprNodePtr& type, const ConstDataNodePtr& data, const Location& loc)
    : DeclNode(NodeType_ConstDecl, loc)
    , name(name)
    , type(type)
    , data(data)
  {}

  ConstDeclNode(const std::string& name, const ConstDataNodePtr& data, const Location& loc)
    : DeclNode(NodeType_ConstDecl, loc)
    , name(name)
    , data(data)
  {}


  void accept(DeclNodeVisitor* visitor) { visitor->visitDecl(this); }

  std::string      name;
  TypeExprNodePtr  type;
  ConstDataNodePtr data;
};


}

#endif // NODE_HPP
