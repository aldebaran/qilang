/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include <qilang/node.hpp>
#include <stdexcept>
#include <string>
#include <sstream>

qiLogCategory("qilang.node");

namespace qilang {

  Node::Node(const std::string &name)
    : name(name)
  {
    qiLogInfo() << "node:" << name;
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
