/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <iostream>
#include <qi/log.hpp>
#include <qilang/node.hpp>
#include <qilang/formatter.hpp>
#include <qi/os.hpp>
#include "formatter_p.hpp"
#include "cpptype.hpp"

qiLogCategory("qigen.hppinterface");

namespace qilang {

//Generate Type Registration Information
class QiLangGenObjectDecl : public NodeVisitor, public NodeFormatter {
public:
  int toclose;
  int id;
  std::string currentParent;

  virtual void accept(const NodePtr& node) { node->accept(this); }

  void formatHeader() {
    indent() << "/*" << std::endl;
    indent() << "** qiLang generated file. DO NOT EDIT" << std::endl;
    indent() << "*/" << std::endl;
  }

  void formatFooter() {
    for (int i = 0; i < toclose; ++i) {
      out() << "}" << std::endl;
    }
  }

protected:
  void visit(PackageNode* node) {
    std::vector<std::string> ns = splitPkgName(node->name->name);
    for (int i = 0; i < ns.size(); ++i) {
      toclose++;
      indent() << "namespace " << ns.at(i) << " {" << std::endl;
    }
    out() << std::endl;
  }

  void visit(ImportNode* node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(IntConstNode *node) {
    out() << node->value;
  }
  void visit(FloatConstNode *node) {
    out() << node->value;
  }
  void visit(StringConstNode *node) {
    out() << node->value;
  }
  void visit(TupleConstNode* node) {
    out() << "(" << "FAIL" << ")";
  }
  void visit(ListConstNode* node) {
    out() << "[" << "FAIL" << "]";
  }
  void visit(DictConstNode* node) {
    out() << "{" << "FAIL" << "}";
  }
  void visit(SymbolNode* node) {
    out() << node->name;
  }
  void visit(BinaryOpNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(UnaryOpNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(VarNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(ExprNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(SimpleTypeNode *node) {
    out() << typeToCpp(node);
  }
  void visit(ListTypeNode *node) {
    out() << typeToCpp(node);
  }
  void visit(MapTypeNode *node) {
    out() << typeToCpp(node);
  }
  void visit(TupleTypeNode *node) {
    out() << typeToCpp(node);
  }

  //indented block
  void scopedDecl(const std::vector<qilang::NodePtr>& vec) {
    ScopedIndent _(_indent);
    for (unsigned int i = 0; i < vec.size(); ++i) {
      vec[i]->accept(this);
    }
  }
  void visit(ObjectNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(PropertyNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(AtNode* node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(InterfaceDeclNode* node) {
    int current = id;
    id++;
    currentParent = node->name->name + "Interface";
    indent() << "static int initType" << current << "() {" << std::endl;
    {
      ScopedIndent _(_indent);
      indent() << "qi::ObjectTypeBuilder<" << currentParent << "> builder;" << std::endl;
      for (unsigned int i = 0; i < node->values.size(); ++i) {
        accept(node->values.at(i));
      }
      currentParent = "";
    }
    indent() << "}" << std::endl;
    indent() << "static int myinittype" << current << " = initType" << current << "()" << std::endl;
    indent() << std::endl;
  }
  void visit(FnDeclNode* node) {
    indent() << "builder.advertiseMethod(\"" << expr(node->name) << "\", &" << currentParent << "::" << expr(node->name);
    out() << ");" << std::endl;
  }
  void visit(InDeclNode* node) {
    indent() << "builder.advertiseMethod(\"" << expr(node->name) << "\", &" << currentParent << "::" << expr(node->name);
    out() << ");" << std::endl;
  }
  void visit(OutDeclNode* node) {
    indent() << "builder.advertiseSignal(\"" << expr(node->name) << "\", &" << currentParent << "::" << expr(node->name);
    out() << ");" << std::endl;
  }
  void visit(PropDeclNode* node) {
    indent() << "builder.advertiseProperty(\"" << expr(node->name) << "\", &" << currentParent << "::" << expr(node->name);
    out() << ");" << std::endl;
  }
  void visit(StructNode* node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(VarDefNode* node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(ConstDefNode* node) {
    throw std::runtime_error("unimplemented");
  }

};


std::string genCppObjectRegistration(const NodePtr& node) {
  return QiLangGenObjectDecl().format(node);
}

std::string genCppObjectRegistration(const NodePtrVector& nodes) {
  return QiLangGenObjectDecl().format(nodes);
}


}
