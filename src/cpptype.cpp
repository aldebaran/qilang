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
static std::string constRefYourSelf(const std::string& type, bool constref=true) {
  if (!constref)
    return type;
  return "const " + type + "&";
}

static std::string typeToCpp(const std::string& type, bool constref=true) {
  const char* pod[] = { "bool", "char", "int",
                        "int8", "uint8",
                        "int16", "uint16",
                        "int32", "uint32",
                        "int64", "uint64",
                        "float32", "float64",
                        0 };
  int i = 0;
  while (pod[i]) {
    if (type == pod[i])
      return type;
    ++i;
  }
  if (type == "str")
    return constRefYourSelf("std::string");
  return constRefYourSelf(type, constref);
}


//void typeToCppVisitor(std::ostream& o, const TypeExprNode& node);

class TypeToCppVisitor: public TypeExprNodeFormatter {
public:
  int noconstref;

  virtual void type(const TypeExprNodePtr& node) { node->accept(this); }

  explicit TypeToCppVisitor(bool noconst) {
    noconstref = 0;
    if (noconst)
      noconstref++;
  }

  const std::string& noconst(TypeExprNodePtr node) {
    static const std::string ret;
    noconstref++;
    type(node);
    noconstref--;
    return ret;
  }

  const std::string& doconst() {
    static const std::string constt("const ");
    static const std::string empt;
    if (noconstref == 0)
      return constt;
    return empt;
  }

  const std::string& doref() {
    static const std::string constt("&");
    static const std::string empt;
    if (noconstref == 0)
      return constt;
    return empt;
  }

  void visit(SimpleTypeExprNode* node) {
    out() << typeToCpp(node->value, noconstref==0);
  }
  void visit(ListTypeExprNode* node) {
    out() << doconst() << "std::vector< " << noconst(node->element) << " >" << doref();
  }
  void visit(MapTypeExprNode* node) {
    out() << doconst() << "std::map< " << noconst(node->key) << ", " << noconst(node->value) << ">" << doref();
  }
  void visit(TupleTypeExprNode* node) {
    out() << "TUPLENOTIMPL";
  }
};

class DataToCppVisitor : public ConstDataNodeVisitor, public ExprNodeFormatter {
public:
  virtual void accept(const ConstDataNodePtr& node) { node->accept(this); }

  void visit(BoolConstDataNode *node) {
    if (node->value)
      out() << "true";
    else
      out() << "false";
  }
  void visit(IntConstDataNode *node) {
    out() << node->value;
  }
  void visit(FloatConstDataNode *node) {
    out() << node->value;
  }
  void visit(StringConstDataNode *node) {
    out() << node->value;
  }
  void visit(TupleConstDataNode* node) {
    out() << "(" << "FAIL" << ")";
  }
  void visit(ListConstDataNode* node) {
    out() << "[" << "FAIL" << "]";
  }
  void visit(DictConstDataNode* node) {
    out() << "{" << "FAIL" << "}";
  }
};

static void noDestroy(TypeExprNode*) {}

std::string typeToCpp(TypeExprNode* type, bool constref) {
  TypeExprNodePtr tnp(type, &noDestroy);
  return TypeToCppVisitor(!constref).format(tnp);
}


std::vector<std::string> splitPkgName(const std::string& name) {
  std::vector<std::string> ret;

  boost::split(ret, name, boost::is_any_of("."));
  return ret;
}

}
