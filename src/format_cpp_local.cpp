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

qiLogCategory("qigen.cpplocal");

namespace qilang {

  //Generate Type Registration Information
  class QiLangGenObjectLocal: public CppTypeFormatter
  {
  public:
    QiLangGenObjectLocal(const PackageManagerPtr& pm, const StringVector& includes)
      : toclose(0)
      , _includes(includes)
    {}

    int toclose;
    StringVector _includes;
    FormatAttr methodAttr;

    virtual void doAccept(Node* node) { node->accept(this); }

    void visitDecl(InterfaceDeclNode* node) {
      indent() << "template<class T>" << std::endl;
      indent() << "class " << node->name + "Local" << ": public " << node->name + ", public T {" << std::endl;
      indent() << "public:" << std::endl;
      {
        ScopedIndent _(_indent);
        ScopedFormatAttrActivate _2(methodAttr);
        for (unsigned int i = 0; i < node->values.size(); ++i) {
          accept(node->values.at(i));
        }
      }
      out() << std::endl;
      indent() << "};" << std::endl;
      indent() << std::endl;
    }

    void visitDecl(ParamFieldDeclNode* node) {

    }

    void visitDecl(FnDeclNode* node) {
      indent() << "";
      accept(node->effectiveRet());
      out() << " " << node->name << "(";
      cppParamsFormat(this, node->args);
      out() << ") {" << std::endl;
      {
        ScopedIndent _(_indent);
        indent() << "return qi::wrapFuture(), T::" << node->name << "(";
        cppParamsFormat(this, node->args, CppParamsFormat_NameOnly);
        out() << ");" << std::endl;
      }
      indent() << "}" << std::endl;
    }

    void visitDecl(SigDeclNode* node) {
      qiLogError() << "SigDeclNode not implemented";
    }
    void visitDecl(PropDeclNode* node) {
      qiLogError() << "PropDeclNode not implemented";
    }
    void visitDecl(StructDeclNode* node) {
      qiLogError() << "StructDeclNode not implemented";
    }
    void visitDecl(StructFieldDeclNode* node) {
      qiLogError() << "FieldDeclNode not implemented";
    }
    void visitDecl(ConstDeclNode* node) {
      qiLogError() << "FieldDeclNode not implemented";
    }
    void visitDecl(EnumDeclNode* node) {
      qiLogError() << "EnumDeclNode not implemented";
    }
    void visitDecl(EnumFieldDeclNode* node) {
      qiLogError() << "EnumFieldDeclNode not implemented";
    }
    void visitDecl(TypeDefDeclNode* node) {
      qiLogError() << "TypeDefDeclNode not implemented";
    }

  void formatHeader() {
    indent() << "/*" << std::endl;
    indent() << "** qiLang generated file. DO NOT EDIT" << std::endl;
    indent() << "*/" << std::endl;
    indent() << "#include <qi/future.hpp>" << std::endl;
    for (unsigned i = 0; i < _includes.size(); ++i) {
      indent() << "#include " << _includes.at(i) << std::endl;
    }
    indent() << std::endl;
  }

  void formatFooter() {
    for (int i = 0; i < toclose; ++i) {
      out() << "}" << std::endl;
    }
  }

protected:
  void visitStmt(PackageNode* node) {
    std::vector<std::string> ns = splitPkgName(node->name);
    for (unsigned int i = 0; i < ns.size(); ++i) {
      toclose++;
      indent() << "namespace " << ns.at(i) << " {" << std::endl;
    }
    out() << std::endl;
  }

  void visitStmt(ImportNode* node) {
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

std::string genCppObjectLocal(const PackageManagerPtr& pm, const ParseResultPtr& pr) {
  StringVector sv = extractCppIncludeDir(pm, pr, true);
  return QiLangGenObjectLocal(pm, sv).format(pr->ast);
}



}
