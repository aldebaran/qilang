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
#include <qi/types.hpp>
#include <boost/shared_ptr.hpp>

namespace qilang {

// AST Node

// Package Management
class PackageNode;
class ImportNode;

// Expression
class IntNode;
class FloatNode;
class StringNode;
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

//pure virtual
class NodeVisitor {
public:
  // Package Management
  virtual void visit(PackageNode* node) = 0;
  virtual void visit(ImportNode* node) = 0;

  // Expression
  virtual void visit(IntNode *node) = 0;
  virtual void visit(FloatNode *node) = 0;
  virtual void visit(StringNode *node) = 0;
  virtual void visit(BinaryOpNode *node) = 0;
  virtual void visit(UnaryOpNode *node) = 0;
  virtual void visit(ExprNode *node) = 0;
  virtual void visit(VarNode *node) = 0;

  // Object Graph
  virtual void visit(ObjectNode *node) = 0;
  virtual void visit(PropertyNode *node) = 0;
  virtual void visit(AtNode *node) = 0;

  // Interface Declaration
  virtual void visit(InterfaceDeclNode* node) = 0;
  virtual void visit(FnDeclNode* node) = 0;
  virtual void visit(InDeclNode* node) = 0;
  virtual void visit(OutDeclNode* node) = 0;
  virtual void visit(PropDeclNode* node) = 0;
};

//Base Node used to describe the AST
class QILANG_API Node
{
public:
  Node(const std::string &name);

  virtual void accept(NodeVisitor *visitor) = 0;

  std::string        name;
};

typedef boost::shared_ptr<Node> NodePtr;

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
  PackageNode(const std::string& packageName)
    : Node("package")
    , name(packageName)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

public:
  std::string name;
};

class QILANG_API ImportNode : public Node {
public:
  ImportNode(const std::string& packageName, const std::vector<std::string>& imported)
    : Node("import")
    , name(packageName)
    , imported(imported)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

public:
  std::string              name;
  std::vector<std::string> imported;
};

class QILANG_API BinaryOpNode : public Node {
public:
  BinaryOpNode(NodePtr n1, NodePtr n2, BinaryOpCode boc)
    : Node("binaryop")
    , op(boc)
    , n1(n1)
    , n2(n2)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

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

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

  UnaryOpCode op;
  NodePtr n1;

};

class QILANG_API IntNode: public Node {
public:
  IntNode(qi::uint64_t val)
    : Node("int")
    , value(val)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

  qi::uint64_t value;
};

class QILANG_API FloatNode: public Node {
public:
  FloatNode(double val)
    : Node("float")
    , value(val)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

  double value;
};

class QILANG_API StringNode: public Node {
public:
  StringNode(const std::string& value)
    : Node("string")
    , value(value)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

  const std::string value;
};

class QILANG_API VarNode : public Node {
public:
  VarNode(const std::string &name)
    : Node("var")
    , value(name)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

  std::string value;
};

class QILANG_API ExprNode : public Node {
public:
  ExprNode(NodePtr child)
    : Node("expr")
    , value(child)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

  NodePtr value;
};

// Object Motion.MoveTo "titi"
class QILANG_API ObjectNode : public Node {
public:
  ObjectNode(const std::string& type, const std::string& id, const std::vector<NodePtr>& defs)
    : Node("object")
    , type(type)
    , id(id)
    , values(defs)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

  std::string          type;
  std::string          id;
  std::vector<NodePtr> values;
};

// myprop: tititoto
class QILANG_API PropertyNode : public Node {
public:
  PropertyNode(const std::string& var, NodePtr value)
    : Node("defprop")
    , var(var)
    , value(value)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

  std::string var;
  NodePtr     value;
};

class QILANG_API AtNode : public Node {
public:
  AtNode(const std::string& sender, const std::string& receiver)
    : Node("at")
    , sender(sender)
    , receiver(receiver)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

public:
  std::string sender;
  std::string receiver;
};

// Object Motion.MoveTo "titi"
class QILANG_API InterfaceDeclNode : public Node {
public:
  InterfaceDeclNode(const std::string& name, const std::vector<NodePtr>& defs)
    : Node("interface")
    , name(name)
    , values(defs)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

  std::string          name;
  std::vector<NodePtr> values;
};

class QILANG_API FnDeclNode : public Node {
public:
  FnDeclNode(const std::string& name, const std::vector<std::string>& args, std::string ret)
    : Node("fn")
    , name(name)
    , args(args)
    , ret(ret)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

public:
  std::string              name;
  std::vector<std::string> args;
  std::string              ret;
};

class QILANG_API InDeclNode : public Node {
public:
  InDeclNode(const std::string& name, const std::vector<std::string>& args)
    : Node("in")
    , name(name)
    , args(args)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

public:
  std::string              name;
  std::vector<std::string> args;
};

class QILANG_API OutDeclNode : public Node {
public:
  OutDeclNode(const std::string& name, const std::vector<std::string>& args)
    : Node("out")
    , name(name)
    , args(args)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

public:
  std::string              name;
  std::vector<std::string> args;
};

class QILANG_API PropDeclNode : public Node {
public:
  PropDeclNode(const std::string& name, const std::vector<std::string>& args)
    : Node("prop")
    , name(name)
    , args(args)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

public:
  std::string              name;
  std::vector<std::string> args;
};


//// ### THIS IS THE FUTURE... and the future is useless right now


class TypeNode: public Node {
public:
  TypeNode()
    : Node("type")
  {}
};

class StmtNode : public Node {
public:
  StmtNode()
    : Node("stmt")
  {}
};

class DeclNode : public Node {
public:
  DeclNode()
    : Node("decl")
  {}
};

class CallNode : public Node {
public:
  CallNode()
    : Node("call")
  {}
};

class ForNode : public Node {
public:
  ForNode()
    : Node("for")
  {}
};

class IfNode : public Node {
public:
  IfNode()
    : Node("if")
  {}
};

class ConstantNode : public Node {
public:
  ConstantNode(const std::string &constant)
    : Node("constant")
  {}
  ConstantNode(const char *constant)
    : Node("constant")
  {}
};

QILANG_API std::string formatAST(NodePtr node);
QILANG_API std::string format(NodePtr node);


}

#endif // NODE_HPP
