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

namespace qilang {

// AST Node

// Package Management
class PackageNode;
class ImportNode;


// Const Expression
class ConstExprNode;
class IntConstNode;
class FloatConstNode;
class StringConstNode;
class ListConstNode;
class DictConstNode;
class TupleConstNode;

// Type
class TypeNode;
class SimpleTypeNode;
class ListTypeNode;
class MapTypeNode;
class TupleTypeNode;

// Expression
class SymbolNode;
class BinaryOpNode;
class UnaryOpNode;
class VarNode;
class ExprNode;

// Object Graphs
class ObjectNode;
class PropertyNode;
class AtNode;

// Interface Declaration
class InterfaceDeclNode;
class FnDeclNode;
class InDeclNode;
class OutDeclNode;
class PropDeclNode;

// Struct
class StructNode;
class VarDefNode;
class ConstDefNode;

//pure virtual
class NodeVisitor {
public:
  // Package Management
  virtual void visit(PackageNode* node) = 0;
  virtual void visit(ImportNode* node) = 0;

  // Expression
  virtual void visit(IntConstNode* node) = 0;
  virtual void visit(FloatConstNode* node) = 0;
  virtual void visit(StringConstNode* node) = 0;
  virtual void visit(TupleConstNode* node) = 0;
  virtual void visit(ListConstNode* node) = 0;
  virtual void visit(DictConstNode* node) = 0;

  virtual void visit(BinaryOpNode* node) = 0;
  virtual void visit(UnaryOpNode* node) = 0;
  virtual void visit(ExprNode* node) = 0;
  virtual void visit(VarNode* node) = 0;
  virtual void visit(SymbolNode* node) = 0;

  // Type
  virtual void visit(SimpleTypeNode* node) = 0;
  virtual void visit(ListTypeNode* node) = 0;
  virtual void visit(MapTypeNode* node) = 0;
  virtual void visit(TupleTypeNode* node) = 0;

  // Object Graph
  virtual void visit(ObjectNode* node) = 0;
  virtual void visit(PropertyNode* node) = 0;
  virtual void visit(AtNode* node) = 0;

  // Interface Declaration
  virtual void visit(InterfaceDeclNode* node) = 0;
  virtual void visit(FnDeclNode* node) = 0;
  virtual void visit(InDeclNode* node) = 0;
  virtual void visit(OutDeclNode* node) = 0;
  virtual void visit(PropDeclNode* node) = 0;

  virtual void visit(VarDefNode* node) = 0;
  virtual void visit(ConstDefNode* node) = 0;
  virtual void visit(StructNode* node) = 0;
};


class TypeNodeVisitor: public NodeVisitor {
  // Type
  virtual void visit(SimpleTypeNode* node) = 0;
  virtual void visit(ListTypeNode* node) = 0;
  virtual void visit(MapTypeNode* node) = 0;
  virtual void visit(TupleTypeNode* node) = 0;

  // We dont mind about everything else
  virtual void visit(PackageNode* node)       { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(ImportNode* node)        { throw std::runtime_error("not implemented for a TypeVisitor"); }

  virtual void visit(IntConstNode* node)      { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(FloatConstNode* node)    { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(StringConstNode* node)   { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(TupleConstNode* node)    { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(ListConstNode* node)     { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(DictConstNode* node)     { throw std::runtime_error("not implemented for a TypeVisitor"); }

  virtual void visit(BinaryOpNode* node)      { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(UnaryOpNode* node)       { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(ExprNode* node)          { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(VarNode* node)           { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(SymbolNode* node)        { throw std::runtime_error("not implemented for a TypeVisitor"); }

  virtual void visit(ObjectNode* node)        { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(PropertyNode* node)      { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(AtNode* node)            { throw std::runtime_error("not implemented for a TypeVisitor"); }

  virtual void visit(InterfaceDeclNode* node) { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(FnDeclNode* node)        { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(InDeclNode* node)        { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(OutDeclNode* node)       { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(PropDeclNode* node)      { throw std::runtime_error("not implemented for a TypeVisitor"); }

  virtual void visit(VarDefNode* node)        { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(ConstDefNode* node)      { throw std::runtime_error("not implemented for a TypeVisitor"); }
  virtual void visit(StructNode* node)        { throw std::runtime_error("not implemented for a TypeVisitor"); }
};


//Base Node used to describe the AST
class QILANG_API Node
{
public:
  Node(const std::string &name);

  virtual void accept(NodeVisitor* visitor) = 0;

  std::string        name;
};

// NodePtr
typedef boost::shared_ptr<Node>           NodePtr;
typedef std::vector<NodePtr>              NodePtrVector;

// SymbolNodePtr
typedef boost::shared_ptr<SymbolNode>     SymbolNodePtr;
typedef std::vector<SymbolNodePtr>        SymbolNodePtrVector;

// TypeNodePtr
typedef boost::shared_ptr<TypeNode>       TypeNodePtr;
typedef std::vector<TypeNodePtr>          TypeNodePtrVector;

// ExprNodePtr
typedef boost::shared_ptr<ExprNode>       ExprNodePtr;
typedef std::vector<ExprNodePtr>          ExprNodePtrVector;

// ConstExprNodePtr
typedef boost::shared_ptr<ConstExprNode>              ConstExprNodePtr;
typedef std::vector<ConstExprNodePtr>                 ConstExprNodePtrVector;
typedef std::pair<ConstExprNodePtr, ConstExprNodePtr> ConstExprNodePtrPair;
typedef std::vector<ConstExprNodePtrPair>             ConstExprNodePtrPairVector;

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

class QILANG_API PackageNode : public Node {
public:
  explicit PackageNode(const SymbolNodePtr& packageName)
    : Node("package")
    , name(packageName)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

public:
  SymbolNodePtr name;
};

class QILANG_API ImportNode : public Node {
public:
  explicit ImportNode(const SymbolNodePtr& packageName)
    : Node("import")
    , name(packageName)
  {}

  ImportNode(const SymbolNodePtr& packageName, const SymbolNodePtrVector& imported)
    : Node("import")
    , name(packageName)
    , imported(imported)
  {
    if (imported.size() == 0)
      throw std::runtime_error("Empty import list");
  }

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

public:
  SymbolNodePtr       name;
  SymbolNodePtrVector imported;
};

class QILANG_API BinaryOpNode : public Node {
public:
  BinaryOpNode(NodePtr n1, NodePtr n2, BinaryOpCode boc)
    : Node("binaryop")
    , op(boc)
    , n1(n1)
    , n2(n2)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  BinaryOpCode op;
  NodePtr        n1;
  NodePtr        n2;
};

class QILANG_API  UnaryOpNode : public Node {
public:
  UnaryOpNode(NodePtr node, UnaryOpCode op)
    : Node("unaryop")
    , op(op)
    , n1(node)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  UnaryOpCode op;
  NodePtr n1;

};

//Virtual
class QILANG_API ConstExprNode : public Node {
public:
  explicit ConstExprNode(const std::string& name)
    : Node(name)
  {}
};

class QILANG_API IntConstNode: public ConstExprNode {
public:
  explicit IntConstNode(qi::uint64_t val)
    : ConstExprNode("int")
    , value(val)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  qi::uint64_t value;
};

class QILANG_API FloatConstNode: public ConstExprNode {
public:
  explicit FloatConstNode(double val)
    : ConstExprNode("float")
    , value(val)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  double value;
};

class QILANG_API StringConstNode: public ConstExprNode {
public:
  explicit StringConstNode(const std::string& value)
    : ConstExprNode("string")
    , value(value)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  const std::string value;
};

class QILANG_API ListConstNode: public ConstExprNode {
public:
  explicit ListConstNode(const ConstExprNodePtrVector& values)
    : ConstExprNode("list")
    , values(values)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  ConstExprNodePtrVector values;
};

class QILANG_API TupleConstNode: public ConstExprNode {
public:
  explicit TupleConstNode(const ConstExprNodePtrVector& values)
    : ConstExprNode("tuple")
    , values(values)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  ConstExprNodePtrVector values;
};

class QILANG_API DictConstNode: public ConstExprNode {
public:
  explicit DictConstNode(const ConstExprNodePtrPairVector& values)
    : ConstExprNode("dict")
    , values(values)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  ConstExprNodePtrPairVector values;
};


class QILANG_API VarNode : public Node {
public:
  explicit VarNode(const SymbolNodePtr &name)
    : Node("var")
    , value(name)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  SymbolNodePtr value;
};

class QILANG_API SymbolNode : public Node {
public:
  explicit SymbolNode(const std::string& symname)
    : Node("symbol")
    , name(symname)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  std::string name;
};

//NOT Visitable
class QILANG_API TypeNode : public Node {
public:
  explicit TypeNode(const std::string& name)
    : Node(name)
  {}
};

class QILANG_API SimpleTypeNode : public TypeNode {
public:
  explicit SimpleTypeNode(const SymbolNodePtr& sym)
    : TypeNode("type")
    , value(sym->name)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  std::string value;
};

class QILANG_API ListTypeNode : public TypeNode {
public:
  explicit ListTypeNode(const TypeNodePtr& element)
    : TypeNode("listtype")
    , element(element)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  TypeNodePtr element;
};

class QILANG_API MapTypeNode : public TypeNode {
public:
  explicit MapTypeNode(const TypeNodePtr& key, const TypeNodePtr& value)
    : TypeNode("maptype")
    , key(key)
    , value(value)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  TypeNodePtr key;
  TypeNodePtr value;
};

class QILANG_API TupleTypeNode : public TypeNode {
public:
  explicit TupleTypeNode(const TypeNodePtrVector& elements)
    : TypeNode("tupletype")
    , elements(elements)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  TypeNodePtrVector elements;
};


class QILANG_API ConstDefNode : public Node {
public:
  ConstDefNode(const SymbolNodePtr& name, const TypeNodePtr& type, const ConstExprNodePtr& value)
    : Node("constdef")
    , name(name)
    , type(type)
    , value(value)
  {}

  ConstDefNode(const SymbolNodePtr& name, const ConstExprNodePtr& value)
    : Node("constdef")
    , name(name)
    , value(value)
  {}


  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  SymbolNodePtr name;
  NodePtr type;
  ConstExprNodePtr     value;
};

class QILANG_API VarDefNode : public Node {
public:
  VarDefNode(const SymbolNodePtr& name, const TypeNodePtr& type, const NodePtr& value)
    : Node("vardef")
    , name(name)
    , type(type)
    , value(value)
  {}

  VarDefNode(const SymbolNodePtr& name, const TypeNodePtr& type)
    : Node("vardef")
    , name(name)
    , type(type)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  SymbolNodePtr    name;
  NodePtr          type;
  NodePtr value;
};



class QILANG_API StructNode : public Node {
public:
  StructNode(const SymbolNodePtr& name, const NodePtrVector& vardefs)
    : Node("struct")
    , name(name)
    , values(vardefs)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  SymbolNodePtr   name;
  NodePtrVector values;
};

class QILANG_API ExprNode : public Node {
public:
  explicit ExprNode(NodePtr child)
    : Node("expr")
    , value(child)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  NodePtr value;
};


// Object Motion.MoveTo "titi"
class QILANG_API ObjectNode : public Node {
public:
  ObjectNode(const TypeNodePtr& type, const ConstExprNodePtr& id, const NodePtrVector& defs)
    : Node("object")
    , type(type)
    , id(id)
    , values(defs)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  TypeNodePtr      type;
  ConstExprNodePtr id;
  NodePtrVector    values;
};

// myprop: tititoto
class QILANG_API PropertyNode : public Node {
public:
  PropertyNode(const SymbolNodePtr& var, NodePtr value)
    : Node("propdef")
    , var(var)
    , value(value)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  SymbolNodePtr var;
  NodePtr       value;
};

class QILANG_API AtNode : public Node {
public:
  AtNode(const NodePtr& sender, const NodePtr& receiver)
    : Node("at")
    , sender(sender)
    , receiver(receiver)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

public:
  NodePtr sender;
  NodePtr receiver;
};

// Object Motion.MoveTo "titi"
class QILANG_API InterfaceDeclNode : public Node {
public:
  InterfaceDeclNode(const SymbolNodePtr& name, const NodePtrVector& defs)
    : Node("interface")
    , name(name)
    , values(defs)
  {}

  InterfaceDeclNode(const SymbolNodePtr& name, const SymbolNodePtrVector& inherits, const NodePtrVector& defs)
    : Node("interface")
    , name(name)
    , values(defs)
    , inherits(inherits)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

  SymbolNodePtr       name;
  NodePtrVector       values;
  SymbolNodePtrVector inherits;
};

class QILANG_API FnDeclNode : public Node {
public:
  FnDeclNode(const SymbolNodePtr& name, const TypeNodePtrVector& args, const TypeNodePtr& ret)
    : Node("fn")
    , name(name)
    , args(args)
    , ret(ret)
  {}

  FnDeclNode(const SymbolNodePtr& name, const TypeNodePtrVector& args)
    : Node("fn")
    , name(name)
    , args(args)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

public:
  SymbolNodePtr       name;
  TypeNodePtrVector   args;
  TypeNodePtr         ret;
};

class QILANG_API InDeclNode : public Node {
public:
  InDeclNode(const SymbolNodePtr& name, const TypeNodePtrVector& args)
    : Node("in")
    , name(name)
    , args(args)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

public:
  SymbolNodePtr       name;
  TypeNodePtrVector   args;
};

class QILANG_API OutDeclNode : public Node {
public:
  OutDeclNode(const SymbolNodePtr& name, const TypeNodePtrVector& args)
    : Node("out")
    , name(name)
    , args(args)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

public:
  SymbolNodePtr     name;
  TypeNodePtrVector args;
};

class QILANG_API PropDeclNode : public Node {
public:
  PropDeclNode(const SymbolNodePtr& name, const TypeNodePtrVector& args)
    : Node("prop")
    , name(name)
    , args(args)
  {}

  void accept(NodeVisitor* visitor) { visitor->visit(this); }

public:
  SymbolNodePtr     name;
  TypeNodePtrVector args;
};


}

#endif // NODE_HPP
