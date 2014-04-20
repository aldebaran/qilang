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


class Node;

// STMT: Package Management
class StmtNode;
class PackageNode;
class ImportNode;
class ObjectDefNode;
class PropertyDefNode;
class AtNode;
class VarDefNode;
class ConstDefNode;

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
class SimpleTypeExprNode;
class ListTypeExprNode;
class MapTypeExprNode;
class TupleTypeExprNode;

// EXPR
class ExprNode;        //VIRTUAL: dep on TypeExpr, ConstData
class BinaryOpExprNode;
class UnaryOpExprNode;
class VarExprNode;

// Interface Declaration
//class DeclNode;          //VIRTUAL
class InterfaceDeclNode;
class FnDeclNode;
class EmitDeclNode;
class PropDeclNode;
class StructDeclNode; //Struct Decl


typedef std::vector<std::string>          StringVector;

typedef boost::shared_ptr<Node>           NodePtr;
typedef std::vector<NodePtr>              NodePtrVector;

//typedef boost::shared_ptr<DeclNode>       DeclNodePtr;
//typedef std::vector<DeclNodePtr>          DeclNodePtrVector;

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
class StmtNodeVisitor {
protected:
public:
  virtual void accept(const StmtNodePtr& node) = 0;

  // Package Management
  virtual void visit(PackageNode* node) = 0;
  virtual void visit(ImportNode* node) = 0;

  // Object Definitions
  virtual void visit(ObjectDefNode* node) = 0;
  virtual void visit(PropertyDefNode* node) = 0;
  virtual void visit(AtNode* node) = 0;

  // Definitions
  virtual void visit(ConstDefNode* node) = 0;
  virtual void visit(VarDefNode* node) = 0;

  // Interface Declaration
  virtual void visit(InterfaceDeclNode* node) = 0;
  virtual void visit(FnDeclNode* node) = 0;
  virtual void visit(EmitDeclNode* node) = 0;
  virtual void visit(PropDeclNode* node) = 0;

  // Struct Declaration
  virtual void visit(StructDeclNode* node) = 0;
  // Call ?
};


class ExprNodeVisitor {
public:
  virtual void accept(const ExprNodePtr& node) = 0;

  // Expr
  virtual void visit(BinaryOpExprNode* node) = 0;
  virtual void visit(UnaryOpExprNode* node) = 0;
  virtual void visit(VarExprNode* node) = 0;
};

/** Const Data Expression Visitor
 */
class ConstDataNodeVisitor {
protected:
public:
  friend class ConstDataNode;
  virtual void accept(const ConstDataNodePtr& node) = 0;

  virtual void visit(BoolConstDataNode* node) = 0;
  virtual void visit(IntConstDataNode* node) = 0;
  virtual void visit(FloatConstDataNode* node) = 0;
  virtual void visit(StringConstDataNode* node) = 0;
  virtual void visit(TupleConstDataNode* node) = 0;
  virtual void visit(ListConstDataNode* node) = 0;
  virtual void visit(DictConstDataNode* node) = 0;
};

/** Type Expression Visitor
 */
class TypeExprNodeVisitor {
protected:
public:
  friend class TypeExprNode;
  virtual void accept(const TypeExprNodePtr& node) = 0;

  virtual void visit(SimpleTypeExprNode* node) = 0;
  virtual void visit(ListTypeExprNode* node) = 0;
  virtual void visit(MapTypeExprNode* node) = 0;
  virtual void visit(TupleTypeExprNode* node) = 0;
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

  NodeType_SimpleTypeExpr,
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
  NodeType_ConstDecl
};

//Base Node used to describe the AST
class QILANG_API Node
{
public:
  Node(NodeKind kind, NodeType type);

  NodeKind kind() const { return _kind; }
  NodeType type() const { return _type; }

private:
  NodeKind _kind;
  NodeType _type;
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
public:
  explicit ExprNode(NodeType type)
    : Node(NodeKind_Expr, type)
  {}

  virtual void accept(ExprNodeVisitor *visitor) = 0;
};

class QILANG_API BinaryOpExprNode : public ExprNode {
public:
  BinaryOpExprNode(ExprNodePtr n1, ExprNodePtr n2, BinaryOpCode boc)
    : ExprNode(NodeType_BinOpExpr)
    , op(boc)
    , n1(n1)
    , n2(n2)
  {}

  void accept(ExprNodeVisitor* visitor) { visitor->visit(this); }

  BinaryOpCode op;
  ExprNodePtr  n1;
  ExprNodePtr  n2;
};

class QILANG_API  UnaryOpExprNode : public ExprNode {
public:
  UnaryOpExprNode(ExprNodePtr node, UnaryOpCode op)
    : ExprNode(NodeType_UOpExpr)
    , op(op)
    , n1(node)
  {}

  void accept(ExprNodeVisitor* visitor) { visitor->visit(this); }

  UnaryOpCode op;
  ExprNodePtr n1;

};

class QILANG_API VarExprNode : public ExprNode {
public:
  explicit VarExprNode(const std::string &name)
    : ExprNode(NodeType_VarExpr)
    , value(name)
  {}

  void accept(ExprNodeVisitor* visitor) { visitor->visit(this); }

  std::string value;
};

// ####################
// # CONST DATA Node
// ####################
class QILANG_API ConstDataNode : public Node {
public:
  explicit ConstDataNode(NodeType type)
    : Node(NodeKind_ConstData, type)
  {}

  virtual void accept(ConstDataNodeVisitor* visitor) = 0;
};

class QILANG_API BoolConstDataNode: public ConstDataNode {
public:
  explicit BoolConstDataNode(bool val)
    : ConstDataNode(NodeType_BoolData)
    , value(val)
  {}

  void accept(ConstDataNodeVisitor* visitor) { visitor->visit(this); }

  bool value;
};

class QILANG_API IntConstDataNode: public ConstDataNode {
public:
  explicit IntConstDataNode(qi::uint64_t val)
    : ConstDataNode(NodeType_IntData)
    , value(val)
  {}

  void accept(ConstDataNodeVisitor* visitor) { visitor->visit(this); }

  qi::uint64_t value;
};

class QILANG_API FloatConstDataNode: public ConstDataNode {
public:
  explicit FloatConstDataNode(double val)
    : ConstDataNode(NodeType_FloatData)
    , value(val)
  {}

  void accept(ConstDataNodeVisitor* visitor) { visitor->visit(this); }

  double value;
};

class QILANG_API StringConstDataNode: public ConstDataNode {
public:
  explicit StringConstDataNode(const std::string& value)
    : ConstDataNode(NodeType_StringData)
    , value(value)
  {}

  void accept(ConstDataNodeVisitor* visitor) { visitor->visit(this); }

  const std::string value;
};

class QILANG_API ListConstDataNode: public ConstDataNode {
public:
  explicit ListConstDataNode(const ConstDataNodePtrVector& values)
    : ConstDataNode(NodeType_ListData)
    , values(values)
  {}

  void accept(ConstDataNodeVisitor* visitor) { visitor->visit(this); }

  ConstDataNodePtrVector values;
};

class QILANG_API TupleConstDataNode: public ConstDataNode {
public:
  explicit TupleConstDataNode(const ConstDataNodePtrVector& values)
    : ConstDataNode(NodeType_TupleData)
    , values(values)
  {}

  void accept(ConstDataNodeVisitor* visitor) { visitor->visit(this); }

  ConstDataNodePtrVector values;
};

class QILANG_API DictConstDataNode: public ConstDataNode {
public:
  explicit DictConstDataNode(const ConstDataNodePtrPairVector& values)
    : ConstDataNode(NodeType_MapData)
    , values(values)
  {}

  void accept(ConstDataNodeVisitor* visitor) { visitor->visit(this); }

  ConstDataNodePtrPairVector values;
};


// ####################
// # TYPE EXPR Node
// ####################
class QILANG_API TypeExprNode : public Node {
public:
  explicit TypeExprNode(NodeType type)
    : Node(NodeKind_TypeExpr, type)
  {}

  virtual void accept(TypeExprNodeVisitor* visitor) = 0;
};

class QILANG_API SimpleTypeExprNode : public TypeExprNode {
public:
  explicit SimpleTypeExprNode(const std::string& sym)
    : TypeExprNode(NodeType_SimpleTypeExpr)
    , value(sym)
  {}

  void accept(TypeExprNodeVisitor* visitor) { visitor->visit(this); }

  std::string value;
};

class QILANG_API ListTypeExprNode : public TypeExprNode {
public:
  explicit ListTypeExprNode(const TypeExprNodePtr& element)
    : TypeExprNode(NodeType_ListTypeExpr)
    , element(element)
  {}

  void accept(TypeExprNodeVisitor* visitor) { visitor->visit(this); }

  TypeExprNodePtr element;
};

class QILANG_API MapTypeExprNode : public TypeExprNode {
public:
  explicit MapTypeExprNode(const TypeExprNodePtr& key, const TypeExprNodePtr& value)
    : TypeExprNode(NodeType_MapTypeExpr)
    , key(key)
    , value(value)
  {}

  void accept(TypeExprNodeVisitor* visitor) { visitor->visit(this); }

  TypeExprNodePtr key;
  TypeExprNodePtr value;
};

class QILANG_API TupleTypeExprNode : public TypeExprNode {
public:
  explicit TupleTypeExprNode(const TypeExprNodePtrVector& elements)
    : TypeExprNode(NodeType_TupleTypeExpr)
    , elements(elements)
  {}

  void accept(TypeExprNodeVisitor* visitor) { visitor->visit(this); }

  TypeExprNodePtrVector elements;
};


// ####################
// # STMT Node
// ####################
class QILANG_API StmtNode : public Node
{
public:
  StmtNode(NodeType type)
    : Node(NodeKind_Stmt, type)
  {}

  virtual void accept(StmtNodeVisitor* visitor) = 0;
};

class QILANG_API PackageNode : public StmtNode {
public:
  explicit PackageNode(const std::string& packageName)
    : StmtNode(NodeType_Package)
    , name(packageName)
  {}

  void accept(StmtNodeVisitor* visitor) { visitor->visit(this); }

public:
  std::string name;
};

class QILANG_API ImportNode : public StmtNode {
public:
  explicit ImportNode(const std::string& packageName)
    : StmtNode(NodeType_Import)
    , name(packageName)
  {}

  ImportNode(const std::string& packageName, const StringVector& imported)
    : StmtNode(NodeType_Import)
    , name(packageName)
    , imported(imported)
  {
    if (imported.size() == 0)
      throw std::runtime_error("Empty import list");
  }

  void accept(StmtNodeVisitor* visitor) { visitor->visit(this); }

public:
  std::string  name;
  StringVector imported;
};


class QILANG_API ConstDefNode : public StmtNode {
public:
  ConstDefNode(const std::string& name, const TypeExprNodePtr& type, const ConstDataNodePtr& data)
    : StmtNode(NodeType_ConstDecl)
    , name(name)
    , type(type)
    , data(data)
  {}

  ConstDefNode(const std::string& name, const ConstDataNodePtr& data)
    : StmtNode(NodeType_ConstDecl)
    , name(name)
    , data(data)
  {}


  void accept(StmtNodeVisitor* visitor) { visitor->visit(this); }

  std::string      name;
  TypeExprNodePtr  type;
  ConstDataNodePtr data;
};

class QILANG_API VarDefNode : public StmtNode {
public:
  VarDefNode(const std::string& name, const TypeExprNodePtr& type, const ConstDataNodePtr& data)
    : StmtNode(NodeType_VarDef)
    , name(name)
    , type(type)
    , data(data)
  {}

  VarDefNode(const std::string& name, const TypeExprNodePtr& type)
    : StmtNode(NodeType_VarDef)
    , name(name)
    , type(type)
  {}

  void accept(StmtNodeVisitor* visitor) { visitor->visit(this); }

  std::string      name;
  TypeExprNodePtr  type;
  ConstDataNodePtr data;
};

// Object Motion.MoveTo "titi"
class QILANG_API ObjectDefNode : public StmtNode {
public:
  ObjectDefNode(const TypeExprNodePtr& type, const ConstDataNodePtr& name, const NodePtrVector& defs)
    : StmtNode(NodeType_ObjectDef)
    , type(type)
    , name(name)
    , values(defs)
  {}

  void accept(StmtNodeVisitor* visitor) { visitor->visit(this); }

  TypeExprNodePtr  type;
  ConstDataNodePtr name;

  NodePtrVector    values;
};

// myprop: tititoto
class QILANG_API PropertyDefNode : public StmtNode {
public:
  PropertyDefNode(const std::string& name, ConstDataNodePtr value)
    : StmtNode(NodeType_PropDef)
    , name(name)
    , value(value)
  {}

  void accept(StmtNodeVisitor* visitor) { visitor->visit(this); }

  std::string      name;
  ConstDataNodePtr value;
};

class QILANG_API AtNode : public StmtNode {
public:
  AtNode(const NodePtr& sender, const NodePtr& receiver)
    : StmtNode(NodeType_At)
    , sender(sender)
    , receiver(receiver)
  {}

  void accept(StmtNodeVisitor* visitor) { visitor->visit(this); }

public:
  NodePtr sender;
  NodePtr receiver;
};

// ####################
// # DECL Node
// ####################
typedef StmtNodeVisitor DeclNodeVisitor;
typedef StmtNode        DeclNode;
//class QILANG_API DeclNode : public StmtNode
//{
//public:
//  DeclNode(NodeType type)
//    : Node(NodeKind_Decl, type)
//  {}

//  virtual void accept(DeclNodeVisitor* visitor) = 0;
//};

class QILANG_API StructDeclNode : public DeclNode {
public:
  StructDeclNode(const std::string& name, const NodePtrVector& vardefs)
    : DeclNode(NodeType_StructDecl)
    , name(name)
    , values(vardefs)
  {}

  void accept(DeclNodeVisitor* visitor) { visitor->visit(this); }

  std::string   name;
  NodePtrVector values;
};

// Object Motion.MoveTo "titi"
class QILANG_API InterfaceDeclNode : public DeclNode {
public:
  InterfaceDeclNode(const std::string& name, const DeclNodePtrVector& decls)
    : DeclNode(NodeType_InterfaceDecl)
    , name(name)
    , values(decls)
  {}

  InterfaceDeclNode(const std::string& name, const StringVector& inherits, const DeclNodePtrVector& decls)
    : DeclNode(NodeType_InterfaceDecl)
    , name(name)
    , values(decls)
    , inherits(inherits)
  {}

  void accept(DeclNodeVisitor* visitor) { visitor->visit(this); }

  std::string       name;
  DeclNodePtrVector values;
  StringVector      inherits;
};

class QILANG_API FnDeclNode : public DeclNode {
public:
  FnDeclNode(const std::string& name, const TypeExprNodePtrVector& args, const TypeExprNodePtr& ret)
    : DeclNode(NodeType_FnDecl)
    , name(name)
    , args(args)
    , ret(ret)
  {}

  FnDeclNode(const std::string& name, const TypeExprNodePtrVector& args)
    : DeclNode(NodeType_FnDecl)
    , name(name)
    , args(args)
  {}

  void accept(DeclNodeVisitor* visitor) { visitor->visit(this); }

public:
  std::string       name;
  TypeExprNodePtrVector   args;
  TypeExprNodePtr         ret;
};


class QILANG_API EmitDeclNode : public DeclNode {
public:
  EmitDeclNode(const std::string& name, const TypeExprNodePtrVector& args)
    : DeclNode(NodeType_EmitDecl)
    , name(name)
    , args(args)
  {}

  void accept(DeclNodeVisitor* visitor) { visitor->visit(this); }

public:
  std::string           name;
  TypeExprNodePtrVector args;
};

class QILANG_API PropDeclNode : public DeclNode {
public:
  PropDeclNode(const std::string& name, const TypeExprNodePtrVector& args)
    : DeclNode(NodeType_PropDecl)
    , name(name)
    , args(args)
  {}

  void accept(DeclNodeVisitor* visitor) { visitor->visit(this); }

public:
  std::string           name;
  TypeExprNodePtrVector args;
};


}

#endif // NODE_HPP
