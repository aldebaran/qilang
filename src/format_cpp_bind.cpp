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

qiLogCategory("qigen.cppbind");

namespace qilang {

//Generate Type Registration Information
class CppBindQiLangGen  : public CppTypeFormatter
{
public:
  CppBindQiLangGen(const PackageManagerPtr& pm, const ParseResultPtr& pr, const StringVector& includes)
    : toclose(0)
    , _includes(includes)
    , _pr(pr)
    , id(0)
  {}

  virtual void doAccept(Node* node) { node->accept(this); }

  int toclose;
  StringVector _includes;
  const ParseResultPtr& _pr;
  FormatAttr methodAttr;

  int id;
  std::string currentParent;

  void formatHeader() {
    indent() << "/*" << std::endl;
    indent() << "** qiLang generated file. DO NOT EDIT" << std::endl;
    indent() << "*/" << std::endl;
    indent() << "#include <qi/type/objecttypebuilder.hpp>" << std::endl;
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

  void visitDecl(InterfaceDeclNode* node) {
    int current = id;
    id++;
    currentParent = formatNs(_pr->package) + "::" + node->name;
    indent() << "static int initType" << current << "() {" << std::endl;
    {
      ScopedIndent _(_indent);
      ScopedFormatAttrActivate _2(methodAttr);
      indent() << "qi::ObjectTypeBuilder< " << currentParent << " > builder;" << std::endl;
      for (unsigned int i = 0; i < node->inherits.size(); ++i) {
        indent() << "builder.inherits< " << node->inherits.at(i) << " >();" << std::endl;
      }
      for (unsigned int i = 0; i < node->values.size(); ++i) {
        accept(node->values.at(i));
      }
      indent() << "builder.registerType();" << std::endl;

      currentParent = "";
      indent() << "return 42;" << std::endl;
    }
    indent() << "}" << std::endl;
    indent() << "static int myinittype" << current << " = initType" << current << "();" << std::endl;
    indent() << std::endl;
  }
  void visitDecl(ParamFieldDeclNode* node) {
    //useless
  }
  void visitDecl(FnDeclNode* node) {
    if (methodAttr.isActive()) {
      indent() << "builder.advertiseMethod(\"" << node->name << "\", static_cast< ";
      accept(node->effectiveRet());
      out() << "(" << currentParent << "::*)(";
      cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
      out() << ") >(&" << currentParent << "::" << node->name;
      out() << "));" << std::endl;
    } else {
      indent() << "//QI_REGISTER_OBJECT_FACTORY(" << node->name << ");" << std::endl;
    }
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
    return;
    indent() << "QI_REGISTER_STRUCT(" << node->name << ", ";
    for (unsigned int i = 0; i < node->decls.size(); ++i) {
      accept(node->decls.at(i));
      if (i + 1 < node->decls.size())
        out() << ", ";
    }
    out() << ");" << std::endl;
  }
  void visitDecl(ConstDeclNode* node) {
    throw std::runtime_error("unimplemented");
  }
  void visitDecl(StructFieldDeclNode* node) {
  }
  void visitDecl(EnumDeclNode* node) {
  }
  void visitDecl(EnumFieldDeclNode* node) {
  }
  void visitDecl(TypeDefDeclNode* node) {
  }

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

std::string genCppObjectRegistration(const PackageManagerPtr& pm, const ParseResultPtr& pr) {
  StringVector sv = extractCppIncludeDir(pm, pr, true);
  return CppBindQiLangGen(pm, pr, sv).format(pr->ast);
}

}
