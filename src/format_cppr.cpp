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
class QiLangGenObjectDecl  : public FileFormatter,
                             public StmtNodeFormatter,
                             public DeclNodeFormatter,
                             public TypeExprNodeFormatter,
                             public ExprNodeFormatter,
                             public ConstDataNodeFormatter {
public:
  int toclose;
  int id;
  std::string currentParent;

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
    std::vector<std::string> ns = splitPkgName(node->name);
    for (int i = 0; i < ns.size(); ++i) {
      toclose++;
      indent() << "namespace " << ns.at(i) << " {" << std::endl;
    }
    out() << std::endl;
  }

  void visit(ImportNode* node) {
    throw std::runtime_error("unimplemented");
  }

  void visit(BinaryOpExprNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(UnaryOpExprNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(VarExprNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(ExprNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(SimpleTypeExprNode *node) {
    out() << typeToCpp(node);
  }
  void visit(ListTypeExprNode *node) {
    out() << typeToCpp(node);
  }
  void visit(MapTypeExprNode *node) {
    out() << typeToCpp(node);
  }
  void visit(TupleTypeExprNode *node) {
    out() << typeToCpp(node);
  }

  void visit(ObjectDefNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(PropertyDefNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(AtNode* node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(InterfaceDeclNode* node) {
    int current = id;
    id++;
    currentParent = node->name + "Interface";
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
    indent() << "builder.advertiseMethod(\"" << node->name << "\", &" << currentParent << "::" << node->name;
    out() << ");" << std::endl;
  }
  void visit(EmitDeclNode* node) {
    indent() << "builder.advertiseSignal(\"" << node->name << "\", &" << currentParent << "::" << node->name;
    out() << ");" << std::endl;
  }
  void visit(PropDeclNode* node) {
    indent() << "builder.advertiseProperty(\"" << node->name << "\", &" << currentParent << "::" << node->name;
    out() << ");" << std::endl;
  }
  void visit(StructDeclNode* node) {
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
