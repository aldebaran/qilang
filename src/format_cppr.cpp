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

  class DeclCppGenFormatter : public DeclNodeFormatter, virtual public CppTypeFormatter, virtual public ExprCppFormatter {
  public:
    int id;
    std::string currentParent;

    virtual void acceptDecl(const DeclNodePtr& node) { node->accept(this); }

    void visitDecl(InterfaceDeclNode* node) {
      int current = id;
      id++;
      currentParent = node->name + "Interface";
      indent() << "static int initType" << current << "() {" << std::endl;
      {
        ScopedIndent _(_indent);
        indent() << "qi::ObjectTypeBuilder<" << currentParent << "> builder;" << std::endl;
        for (unsigned int i = 0; i < node->values.size(); ++i) {
          decl(node->values.at(i));
        }
        currentParent = "";
      }
      indent() << "}" << std::endl;
      indent() << "static int myinittype" << current << " = initType" << current << "()" << std::endl;
      indent() << std::endl;
    }
    void visitDecl(FnDeclNode* node) {
      indent() << "builder.advertiseMethod(\"" << node->name << "\", &" << currentParent << "::" << node->name;
      out() << ");" << std::endl;
    }
    void visitDecl(EmitDeclNode* node) {
      indent() << "builder.advertiseSignal(\"" << node->name << "\", &" << currentParent << "::" << node->name;
      out() << ");" << std::endl;
    }
    void visitDecl(PropDeclNode* node) {
      indent() << "builder.advertiseProperty(\"" << node->name << "\", &" << currentParent << "::" << node->name;
      out() << ");" << std::endl;
    }
    void visitDecl(StructDeclNode* node) {
      throw std::runtime_error("unimplemented");
    }
    void visitDecl(ConstDeclNode* node) {
      throw std::runtime_error("unimplemented");
    }
    void visitDecl(FieldDeclNode* node) {
      throw std::runtime_error("unimplemented");
    }
  };

//Generate Type Registration Information
class QiLangGenObjectDecl  : public FileFormatter,
                             public StmtNodeFormatter,
                             public DeclCppGenFormatter
{
public:
  int toclose;

  virtual void acceptStmt(const StmtNodePtr& node) { node->accept(this); }

  virtual void accept(const NodePtr& node) {
    switch (node->kind()) {
    case NodeKind_ConstData:
      acceptData(boost::dynamic_pointer_cast<ConstDataNode>(node));
      break;
    case NodeKind_Decl:
      acceptDecl(boost::dynamic_pointer_cast<DeclNode>(node));
      break;
    case NodeKind_Expr:
      acceptExpr(boost::dynamic_pointer_cast<ExprNode>(node));
      break;
    case NodeKind_Stmt:
      acceptStmt(boost::dynamic_pointer_cast<StmtNode>(node));
      break;
    case NodeKind_TypeExpr:
      acceptTypeExpr(boost::dynamic_pointer_cast<TypeExprNode>(node));
      break;
    }
  }


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
  void visitStmt(PackageNode* node) {
    std::vector<std::string> ns = splitPkgName(node->name);
    for (int i = 0; i < ns.size(); ++i) {
      toclose++;
      indent() << "namespace " << ns.at(i) << " {" << std::endl;
    }
    out() << std::endl;
  }

  void visitStmt(ImportNode* node) {
    throw std::runtime_error("unimplemented");
  }

  void visitStmt(ObjectDefNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visitStmt(PropertyDefNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visitStmt(AtNode* node) {
    throw std::runtime_error("unimplemented");
  }
  void visitStmt(VarDefNode* node) {
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
