/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <qilang/node.hpp>
#include <stdexcept>
#include <string>
#include <sstream>

namespace qilang {

  class QiParserSExprPrinter : public NodeVisitor {
  public:
    std::string go(Node *node) {
      node->accept(this);
      return ss.str();
    }

    std::stringstream ss;

  protected:
    void visit(IntNode *node) {
      ss << "(int " << node->value << ")";
    }
    void visit(FloatNode *node) {
      ss << "(float " << node->value << ")";
    }
    void visit(StringNode *node) {
      ss << "(string " << node->value << ")";
    }
    void visit(BinaryOpNode *node) {
      ss << "(" << BinaryOpCodeToString(node->op) << " " << toSExpr(node->n1) << " " << toSExpr(node->n2) << ")";
    }
    void visit(UnaryOpNode *node) {
      ss << "(" << UnaryOpCodeToString(node->op) << " " << toSExpr(node->n1) << ")";
    }
    void visit(VarNode *node) {
      ss << "(var " << node->value << ")";
    }
  };

  std::string toSExpr(Node *node) {
    return QiParserSExprPrinter().go(node);
  }


  const std::string &UnaryOpCodeToString(UnaryOpCode op) {
    static std::string minus("-");
    static std::string negate("!");
    switch(op) {
    case UnaryOpCode_Minus:
      return minus;
    case UnaryOpCode_Negate:
      return negate;
    }
    throw std::runtime_error("invalid unary op code");
  }

  const std::string &BinaryOpCodeToString(BinaryOpCode op) {
    static std::string boolor("||");
    static std::string booland("&&");
    static std::string sand("&");
    static std::string sor("|");
    static std::string sxor("^");
    static std::string eqeq("==");
    static std::string ne("!=");
    static std::string gt(">");
    static std::string lt("<");
    static std::string ge(">=");
    static std::string le("<=");
    static std::string plus("+");
    static std::string minus("-");
    static std::string divide("/");
    static std::string multiply("*");
    static std::string modulus("%");
    static std::string fetcharray("[]");

    switch(op) {
    case BinaryOpCode_BoolOr:
      return boolor;
    case BinaryOpCode_BoolAnd:
      return booland;
    case BinaryOpCode_And:
      return sand;
    case BinaryOpCode_Or:
      return sor;
    case BinaryOpCode_Xor:
      return sxor;
    case BinaryOpCode_EqEq:
      return eqeq;
    case BinaryOpCode_Ne:
      return ne;
    case BinaryOpCode_Gt:
      return gt;
    case BinaryOpCode_Lt:
      return lt;
    case BinaryOpCode_Ge:
      return ge;
    case BinaryOpCode_Le:
      return le;
    case BinaryOpCode_Plus:
      return plus;
    case BinaryOpCode_Minus:
      return minus;
    case BinaryOpCode_Divide:
      return divide;
    case BinaryOpCode_Multiply:
      return multiply;
    case BinaryOpCode_Modulus:
      return modulus;
    case BinaryOpCode_FetchArray:
      return fetcharray;
    }
    throw std::runtime_error("invalid binary op code");
  }

}
