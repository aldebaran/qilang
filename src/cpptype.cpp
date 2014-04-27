/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Cedric GESTES
*/
#include <sstream>
#include "cpptype.hpp"
#include "formatter_p.hpp"
#include <boost/algorithm/string.hpp>

namespace qilang {

static std::string constRefYourSelf(const std::string& type, bool constref) {
  if (!constref)
    return type;
  return "const " + type + "&";
}

static std::string builtinTypeToCpp(BuiltinType type, bool constref) {
  switch (type) {
    case BuiltinType_Bool:
      return "bool";
    case BuiltinType_Char:
      return "char";
    case BuiltinType_Int:
      return "int";
    case BuiltinType_UInt:
      return "unsigned int";
    case BuiltinType_Int8:
      return "qi::int8_t";
    case BuiltinType_UInt8:
      return "qi::uint8_t";
    case BuiltinType_Int16:
      return "qi::int16_t";
    case BuiltinType_UInt16:
      return "qi::uint16_t";
    case BuiltinType_Int32:
      return "qi::int32_t";
    case BuiltinType_UInt32:
      return "qi::uint32_t";
    case BuiltinType_Int64:
      return "qi::int64_t";
    case BuiltinType_UInt64:
      return "qi::uint64_t";
    case BuiltinType_Float:
      return "float";
    case BuiltinType_Float32:
      return "float";
    case BuiltinType_Float64:
      return "double";
    case BuiltinType_String:
      return constRefYourSelf("std::string", constref);
    case BuiltinType_Value:
      return constRefYourSelf("qi::AnyValue", constref);
    case BuiltinType_Object:
      return constRefYourSelf("qi::AnyObject", constref);
  }
}


void CppTypeFormatter::acceptTypeExpr(const TypeExprNodePtr& node) {
  node->accept(this);
}


CppTypeFormatter::CppTypeFormatter()
  : addconstref(0)
  , noconstref(0)
{
}

const std::string& CppTypeFormatter::noconst(TypeExprNodePtr node) {
  static const std::string ret;
  noconstref++;
  type(node);
  noconstref--;
  return ret;
}

const std::string& CppTypeFormatter::consttype(const TypeExprNodePtr& node) {
  static const std::string ret;
  addconstref++;
  acceptTypeExpr(node);
  addconstref--;
  return ret;
}

const std::string& CppTypeFormatter::addconst() {
  static const std::string constt("const ");
  static const std::string empt;
  if (addconstref && noconstref==0)
    return constt;
  return empt;
}

const std::string& CppTypeFormatter::addref() {
  static const std::string constt("&");
  static const std::string empt;
  if (addconstref && noconstref==0)
    return constt;
  return empt;
}

void CppTypeFormatter::visitTypeExpr(BuiltinTypeExprNode* node) {
  out() << builtinTypeToCpp(node->builtinType, (noconstref==0 && addconstref));
}
void CppTypeFormatter::visitTypeExpr(CustomTypeExprNode* node) {
  out() << constRefYourSelf(node->value, (noconstref==0 && addconstref));
}
void CppTypeFormatter::visitTypeExpr(ListTypeExprNode* node) {
  out() << addconst() << "std::vector< " << noconst(node->element) << " >" << addref();
}
void CppTypeFormatter::visitTypeExpr(MapTypeExprNode* node) {
  out() << addconst() << "std::map< " << noconst(node->key) << ", " << noconst(node->value) << " >" << addref();
}
void CppTypeFormatter::visitTypeExpr(TupleTypeExprNode* node) {
  out() << "TUPLENOTIMPL";
}

void DataCppFormatter::acceptData(const ConstDataNodePtr& node) {
  node->accept(this);
}

void DataCppFormatter::visitData(BoolConstDataNode *node) {
  if (node->value)
    out() << "true";
  else
    out() << "false";
}
void DataCppFormatter::visitData(IntConstDataNode *node) {
  out() << node->value;
}
void DataCppFormatter::visitData(FloatConstDataNode *node) {
  out() << node->value;
}
void DataCppFormatter::visitData(StringConstDataNode *node) {
  out() << node->value;
}
void DataCppFormatter::visitData(TupleConstDataNode* node) {
  out() << "(" << "FAIL" << ")";
}
void DataCppFormatter::visitData(ListConstDataNode* node) {
  out() << "[" << "FAIL" << "]";
}
void DataCppFormatter::visitData(DictConstDataNode* node) {
  out() << "{" << "FAIL" << "}";
}

void ExprCppFormatter::visitExpr(BinaryOpExprNode *node) {
  throw std::runtime_error("unimplemented");
}
void ExprCppFormatter::visitExpr(UnaryOpExprNode *node) {
  throw std::runtime_error("unimplemented");
}
void ExprCppFormatter::visitExpr(VarExprNode *node) {
  throw std::runtime_error("unimplemented");
}
void ExprCppFormatter::visitExpr(ConstDataExprNode* node) {
  throw std::runtime_error("unimplemented");
}

std::vector<std::string> splitPkgName(const std::string& name) {
  std::vector<std::string> ret;

  boost::split(ret, name, boost::is_any_of("."));
  return ret;
}

std::string pkgNameToDir(const std::string& name) {
  std::string ret(name);
  boost::replace_all(ret, ".", "/");
  return ret;
}

std::string formatNs(const std::string& package) {
  std::string ret;
  std::vector<std::string> v;

  boost::split(v, package, boost::is_any_of("."));

  if (v.size() > 0)
    ret += "::";
  for (unsigned int i = 0; i < v.size(); ++i) {
    ret += v.at(i);
    if (i + 1 < v.size())
      ret += "::";
  }
  return ret;
}


}
