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
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "cpptype.hpp"

qiLogCategory("qigen.hppinterface");


namespace qilang {

class DeclCppIDLFormatter : public DeclNodeFormatter, virtual public CppTypeFormatter, virtual public ExprCppFormatter {
public:
  virtual void acceptDecl(const DeclNodePtr& node) { node->accept(this); }

  void visitDecl(InterfaceDeclNode* node) {
    indent() << "class " << node->name << "Interface";
    if (node->inherits.size() > 0) {
      out() << ": ";
      for (int i = 0; i < node->inherits.size(); ++i) {
        out() << "public " << node->inherits.at(i);
        if (i + 1 != node->inherits.size())
          out() << ", ";
      }
    }
    out() << " {" << std::endl;
    indent() << "public:" << std::endl;
    scopedDecl(node->values);
    indent() << "};" << std::endl << std::endl;
    indent() << "typedef qi::Object<" << node->name << "Interface> " << node->name << ";" << std::endl;
  }

  void visitDecl(FnDeclNode* node) {
    if (node->ret)
      indent() << type(node->ret) << " " << node->name << "(";
    else
      indent() << "void " << node->name << "(";

    for (unsigned int i = 0; i < node->args.size(); ++i) {
      out() << consttype(node->args[i]);
      if (i+1 < node->args.size()) {
        out() << ", ";
      }
    }
    out() << ");" << std::endl;
  }

  void visitDecl(EmitDeclNode* node) {
    indent() << "qi::Signal<";
    for (unsigned int i = 0; i < node->args.size(); ++i) {
      out() << type(node->args[i]);
      if (i+1 < node->args.size()) {
        out() << ", ";
      }
    }
    out() << "> " << node->name << ";" << std::endl;
  }
  void visitDecl(PropDeclNode* node) {
    indent() << "qi::Property<";
    for (unsigned int i = 0; i < node->args.size(); ++i) {
      out() << type(node->args[i]);
      if (i+1 < node->args.size()) {
        out() << ", ";
      }
    }
    out() << "> " << node->name << ";" << std::endl;
  }

  void visitDecl(StructDeclNode* node) {
    indent() << "struct " << node->name << " {" << std::endl;
    noconstref++;
    scopedField(node->fields);
    noconstref--;
    indent() << "};" << std::endl << std::endl;
  }

  void visitDecl(ConstDeclNode* node) {
    indent() << "const ";
    if (node->type)
      out() << type(node->type) << " " << node->name;
    else
      out() << "qi::AnyValue " << node->name;
    if (node->data)
      out() << " = " << cdata(node->data);
    out() << ";" << std::endl;
  }

  void visitDecl(FieldDeclNode* node) {
    if (node->type)
      indent() << type(node->type) << " " << node->name;
    else
      indent() << "qi::AnyValue " << node->name;
    out() << ";" << std::endl;
  }

};

class QiLangGenObjectDef : public FileFormatter
                         , public DeclCppIDLFormatter
                         , public StmtNodeFormatter
{
public:
  QiLangGenObjectDef()
    : toclose(0)
  {}

  int toclose;     //number of } to close (namespace)

  virtual void acceptStmt(const StmtNodePtr &node) { node->accept(this); }

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
    indent() << "#pragma once" << std::endl;
    boost::uuids::uuid u = boost::uuids::random_generator()();
    std::string uuid = boost::uuids::to_string(u);
    indent() << "#ifndef " << uuid << std::endl;
    indent() << "#define " << uuid << std::endl;
    indent() << std::endl;
    indent() << "#include <qitype/signal.hpp>" << std::endl;
    indent() << "#include <qitype/property.hpp>" << std::endl;
    indent() << std::endl;
  }

  void formatFooter() {
    for (int i = 0; i < toclose; ++i) {
      out() << "}" << std::endl;
    }
    out() << std::endl;
    indent() << "#endif" << std::endl;
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
    if (node->type)
      indent() << type(node->type) << " " << node->name;
    else
      indent() << "qi::AnyValue " << node->name;
    if (node->data)
      out() << " = " << cdata(node->data);
    out() << ";" << std::endl;
  }
};


std::string genCppObjectInterface(const NodePtr& node) {
  return QiLangGenObjectDef().format(node);
}

std::string genCppObjectInterface(const NodePtrVector& nodes) {
  return QiLangGenObjectDef().format(nodes);
}

}
