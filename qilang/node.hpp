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
#include <qi/types.hpp>

namespace qilang {

//AST Node

class IntNode;
class FloatNode;
class StringNode;
class BinaryOpNode;
class UnaryOpNode;
class VarNode;


//pure virtual
class NodeVisitor {
public:
  virtual void visit(IntNode *node) = 0;
  virtual void visit(FloatNode *node) = 0;
  virtual void visit(StringNode *node) = 0;
  virtual void visit(BinaryOpNode *node) = 0;
  virtual void visit(UnaryOpNode *node) = 0;
  virtual void visit(VarNode *node) = 0;
};

//Base Node used to describe the AST
class QILANG_API Node
{
public:
  Node(const std::string &name)
    : name(name)
  {}

  virtual void accept(NodeVisitor *visitor) = 0;

  std::string        name;
};

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

class QILANG_API BinaryOpNode : public Node {
public:
  BinaryOpNode(Node *n1, Node *n2, BinaryOpCode boc)
    : Node("binaryop")
    , op(boc)
    , n1(n1)
    , n2(n2)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

  BinaryOpCode op;
  Node*        n1;
  Node*        n2;
};

class QILANG_API  UnaryOpNode : public Node {
public:
  UnaryOpNode(Node *node, UnaryOpCode op)
    : Node("unaryop")
    , op(op)
    , n1(node)
  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

  UnaryOpCode op;
  Node* n1;

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
  StringNode(const char *value)
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
//  VarNode(const std::string &name, TypeNode* type)
//    : Node("var")
//  {}
//  VarNode(const std::string &name, TypeNode* type, ExprNode* value)
//    : Node("var")
//  {}

  void accept(NodeVisitor *visitor) { visitor->visit(this); }

  std::string value;
};


//// ### THIS IS THE FUTURE... and the future is useless right now

class PackageNode : public Node {
public:
  PackageNode()
    : Node("package")
  {}
};

class ExprNode : public Node {
public:
  ExprNode()
    : Node("expr")
  {}

};

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


QILANG_API std::string toSExpr(Node *node);


}

#endif // NODE_HPP
