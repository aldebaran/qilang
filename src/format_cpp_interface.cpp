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
#include <qilang/packagemanager.hpp>
#include <qi/os.hpp>
#include "formatter_p.hpp"
#include <boost/algorithm/string/replace.hpp>
#include <boost/lambda/lambda.hpp>
#include "cpptype.hpp"

qiLogCategory("qigen.hppinterface");


namespace qilang {

class ScopedNamespaceEscaper {
public:
  ScopedNamespaceEscaper(std::ostream& out, const StringVector& ns)
    : out(out)
    , currentNs(ns)
  {
    for (int i = 0; i < currentNs.size(); ++i) {
      out << "}" << std::endl;
    }
    out << std::endl;
  }

  ~ScopedNamespaceEscaper() {
    unsigned int indent = 0;
    for (unsigned int i = 0; i < currentNs.size(); ++i) {
      for (unsigned int j = 0; j < indent; ++j) {
        out << "  ";
      }
      out << "namespace " << currentNs.at(i) << " {" << std::endl;
      indent += 1;
    }
    out << std::endl;
  }

  std::ostream& out;
  StringVector currentNs;
};

class QiLangGenObjectDef: public CppTypeFormatter
{
public:
  QiLangGenObjectDef(const PackageManagerPtr& pm, const ParseResultPtr& pr, const StringVector& includes)
    : _pm(pm)
    , _pr(pr)
    , _includes(includes)
  {
    apiExport = pkgNameToAPI(pr->package);
    //force api export activation by default
    apiAttr.activate();
  }

  FormatAttr  virtualAttr;
  FormatAttr  apiAttr;
  std::string apiExport;

  virtual void doAccept(Node* node) { node->accept(this); }

  void visitDecl(InterfaceDeclNode* node) {
    ScopedFormatAttrActivate _(virtualAttr);
    ScopedFormatAttrBlock    _2(apiAttr);

    indent() << "class " << apiExport << " " << node->name;
    if (node->inherits.size() > 0) {
      out() << ": ";
      for (unsigned int i = 0; i < node->inherits.size(); ++i) {
        out() << "virtual public " << node->inherits.at(i);
        if (i + 1 != node->inherits.size())
          out() << ", ";
      }
    }
    out() << " {" << std::endl;
    indent() << "public:" << std::endl;
    //add a virtual destructor
    indent() << "  virtual ~" << node->name << "() {}" << std::endl;
    scoped(node->values);
    indent() << "};" << std::endl << std::endl;
    indent() << "typedef qi::Object<" << node->name << "> " << node->name << "Ptr;" << std::endl;
  }

  void visitDecl(ParamFieldDeclNode* node) {
    //useless
  }

  void visitDecl(FnDeclNode* node) {
    indent() << apiAttr(apiExport + " ") << virtualAttr("virtual ");
    accept(node->effectiveRet());
    out() << " " << node->name << "(";
    cppParamsFormat(this, node->args);
    out() << ")" << virtualAttr(" = 0") << ";" << std::endl;
  }

  void visitDecl(SigDeclNode* node) {
    indent() << "qi::Signal< ";
    ScopedFormatAttrBlock _(constattr);
    cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
    out() << " > " << node->name << ";" << std::endl;
  }
  void visitDecl(PropDeclNode* node) {
    indent() << "qi::Property< ";
    ScopedFormatAttrBlock _(constattr);
    cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
    out() << " > " << node->name << ";" << std::endl;
  }

  void visitDecl(StructDeclNode* node) {
    indent() << "struct " << node->name << " {" << std::endl;
    ScopedFormatAttrBlock _(constattr);
    scoped(node->decls);
    StringVector fields;
    {
      ScopedIndent _i(_indent);
      out() << std::endl;
      indent() << node->name << "()" << std::endl;
      for (unsigned int i = 0; i < node->decls.size(); ++i) {
        if (StructFieldDeclNodePtr field =
            boost::dynamic_pointer_cast<StructFieldDeclNode>(node->decls[i])) {
          for (unsigned int j = 0; j < field->names.size(); ++j)
            fields.push_back(field->names[j]);
        }
      }
      if (!fields.empty()) {
        indent() << ": ";
        StringVector fieldInits;
        std::transform(fields.begin(), fields.end(),
            std::back_inserter(fieldInits), boost::lambda::_1 + "()");
        join(fieldInits, ", ");
      }
      out() << std::endl;
      indent() << "{}" << std::endl;
    }
    indent() << "};" << std::endl << std::endl;


    {
      ScopedNamespaceEscaper _e(out(), currentNs);
      out() << "QI_TYPE_STRUCT(";
      for (unsigned int i = 0; i < currentNs.size(); ++i) {
        out() << "::" << currentNs.at(i);
      }
      out() << "::" << node->name;
      StringVector fieldRegs;
      std::transform(fields.begin(), fields.end(),
          std::back_inserter(fieldRegs), ", " + boost::lambda::_1);
      join(fieldRegs, "");
      out() << ")" << std::endl;
      out() << std::endl;
    }
  }

  void visitDecl(ConstDeclNode* node) {
    indent() << "const ";
    accept(node->effectiveType());
    out() << " " << node->name;
    if (node->data) {
      out() << " = ";
      accept(node->data);
    }
    out() << ";" << std::endl;
  }

  void visitDecl(StructFieldDeclNode* node) {
    for (unsigned i = 0; i < node->names.size(); ++i) {
      indent();
      accept(node->effectiveType());
      out() << " " << node->names.at(i);
      out() << ";" << std::endl;
    }
  }
  void visitDecl(EnumDeclNode* node) {
    indent() << "enum " << node->name << " {" << std::endl;
    scoped(node->fields);
    indent() << "};" << std::endl << std::endl;
    {
      ScopedNamespaceEscaper _e(out(), currentNs);
      out() << "QI_TYPE_ENUM_REGISTER(";
      for (unsigned int i = 0; i < currentNs.size(); ++i) {
        out() << "::" << currentNs.at(i);
      }
      out() << "::" << node->name << ")" << std::endl << std::endl;
    }
  }
  void visitDecl(EnumFieldDeclNode* node) {
    if (node->fieldType == EnumFieldType_Const) {
      ConstDeclNode* tnode = static_cast<ConstDeclNode*>(node->node.get());
      indent() << tnode->name << " = ";
      accept(tnode->data);
      out() << "," << std::endl;
      return;
    }
    throw std::runtime_error("type in enum not supported in cppi atm");
  }
  void visitDecl(TypeDefDeclNode* node) {
    indent() << "typedef ";
    accept(node->type);
    out() << " " << node->name << ";" << std::endl;
  }

  int toclose;
  StringVector currentNs;
  PackageManagerPtr  _pm;
  const ParseResultPtr& _pr;
  StringVector       _includes;

  void formatHeader() {
    indent() << "/*" << std::endl;
    indent() << "** qiLang generated file. DO NOT EDIT" << std::endl;
    indent() << "*/" << std::endl;
    indent() << "#pragma once" << std::endl;
    std::string headGuard = filenameToCppHeaderGuard(_pr->package, _pr->filename);
    indent() << "#ifndef " << headGuard << std::endl;
    indent() << "#define " << headGuard << std::endl;
    indent() << std::endl;
    for (unsigned i = 0; i < _includes.size(); ++i) {
      indent() << "#include " << _includes.at(i) << std::endl;
    }
    indent() << std::endl;
  }

  void formatFooter() {
    for (int i = 0; i < currentNs.size(); ++i) {
      out() << "}" << std::endl;
    }
    out() << std::endl;
    indent() << "#endif" << std::endl;
  }

protected:
  void visitStmt(PackageNode* node) {
    currentNs = splitPkgName(node->name);
    for (unsigned int i = 0; i < currentNs.size(); ++i) {
      indent() << "namespace " << currentNs.at(i) << " {" << std::endl;
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
    indent();
    accept(node->effectiveType());
    out() << " " << node->name;
    if (node->data) {
      out() << " = ";
      accept(node->data);
    }
    out() << ";" << std::endl;
  }

};

std::string genCppObjectInterface(const PackageManagerPtr& pm, const ParseResultPtr& pr) {
  StringVector sv = extractCppIncludeDir(pm, pr, false);
  return QiLangGenObjectDef(pm, pr, sv).format(pr->ast);
}

}
