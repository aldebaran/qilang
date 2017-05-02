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
#include <qilang/visitor.hpp>
#include <qilang/packagemanager.hpp>
#include <qi/os.hpp>
#include "formatter_p.hpp"
#include <boost/algorithm/string/replace.hpp>
#include <boost/lambda/lambda.hpp>
#include "cpptype.hpp"

qiLogCategory("qigen.hppinterface");


namespace qilang {

class QiLangGenAsyncIface: public CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >
{
public:
  QiLangGenAsyncIface(std::stringstream& ss, std::string api)
    : CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >(ss)
    , apiExport(api)
  {}

  void visitDecl(InterfaceDeclNode* node) {
    ScopedFormatAttrActivate _(virtualAttr);
    ScopedFormatAttrBlock    _2(apiAttr);

    this->indent() << "class " << node->name << "Async";
    if (node->inherits.size() > 0) {
      out() << ": ";
      for (unsigned int i = 0; i < node->inherits.size(); ++i) {
        out() << "virtual public " << node->inherits.at(i) << "Async";
        if (i + 1 != node->inherits.size())
          out() << ", ";
      }
    }
    out() << " {" << std::endl;
    indent() << "public:" << std::endl;
    //add a virtual destructor
    indent() << "  virtual ~" << node->name << "Async() {}" << std::endl;
    scoped(node->values);
    indent() << "};" << std::endl << std::endl;
  }

  void visitDecl(ParamFieldDeclNode* node) {
    //useless
  }

  void visitDecl(FnDeclNode* node) {
    indent() << apiAttr(apiExport + " ") << virtualAttr("virtual ");
    out() << "::qi::Future< ";
    NodeFormatter::accept(node->effectiveRet());
    out() << " > " << node->name << "(";
    cppParamsFormat(this, node->args);
    out() << ")" << virtualAttr(" = 0") << ";" << std::endl;
  }

  void visitDecl(SigDeclNode* node) {
  }
  void visitDecl(PropDeclNode* node) {
  }

  FormatAttr  virtualAttr;
  FormatAttr  apiAttr;
  std::string apiExport;
};

class QiLangGenIfaceSigPropParam: public CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >
{
  bool first;

public:
  QiLangGenIfaceSigPropParam(std::stringstream& ss)
    : CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >(ss)
    , first(true)
  {}

  void visitDecl(FnDeclNode* node) {}
  void visitDecl(SigDeclNode* node) {
    if (first)
      first = false;
    else
      out() << ", ";
    out() << "::qi::Signal< ";
    ScopedFormatAttrBlock _(constattr);
    cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
    out() << " >& " << node->name;
  }
  void visitDecl(PropDeclNode* node) {
    if (first)
      first = false;
    else
      out() << ", ";
    out() << "::qi::Property< ";
    ScopedFormatAttrBlock _(constattr);
    cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
    out() << " >& " << node->name;
  }
};

class QiLangGenIfaceSigPropParamInit: public CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >
{
  bool first;

public:
  QiLangGenIfaceSigPropParamInit(std::stringstream& ss, int indent)
    : CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >(ss, indent)
    , first(true)
  {}

  void visitDecl(FnDeclNode* node) {}
  void visitDecl(SigDeclNode* node) {
    if (first)
    {
      first = false;
      indent() << ": ";
    }
    else
      indent() << ", ";
    out() << node->name << "(" << node->name << ")" << std::endl;
  }
  void visitDecl(PropDeclNode* node) {
    if (first)
    {
      first = false;
      indent() << ": ";
    }
    else
      out() << ", ";
    out() << node->name << "(" << node->name << ")" << std::endl;
  }
  void visitDecl(ParamFieldDeclNode* node) {
    //useless
  }
};

class QiLangGenIface: public CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >
{
public:
  QiLangGenIface(std::stringstream& ss, std::string api)
    : CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >(ss)
    , apiExport(api)
  {}

  void visitDecl(InterfaceDeclNode* node) {
    ScopedFormatAttrActivate _(virtualAttr);
    ScopedFormatAttrBlock    _2(apiAttr);

    indent() << "class " << node->name;
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
    {
      ScopedIndent _i(_indent);
      indent() << node->name << "(";
      {
        QiLangGenIfaceSigPropParam sigprop(out());
        for (unsigned int i = 0; i < node->values.size(); ++i) {
          sigprop.accept(node->values.at(i));
        }
      }
      out() << ")" << std::endl;
      {
        QiLangGenIfaceSigPropParamInit sigpropinit(out(), _indent);
        sigpropinit.scoped(node->values);
      }
      indent() << "{}" << std::endl;
      indent() << "virtual ~" << node->name << "() {}" << std::endl;
      indent() << "virtual " << node->name << "Async& async() = 0;" << std::endl;
    }
    scoped(node->values);

    indent() << "};" << std::endl << std::endl;
  }

  void visitDecl(ParamFieldDeclNode* node) {
    //useless
  }

  void visitDecl(FnDeclNode* node) {
    indent() << apiAttr(apiExport + " ") << virtualAttr("virtual ");
    NodeFormatter::accept(node->effectiveRet());
    out() << " " << node->name << "(";
    cppParamsFormat(this, node->args);
    out() << ")" << virtualAttr(" = 0") << ";" << std::endl;
  }

  void visitDecl(SigDeclNode* node) {
    ScopedFormatAttrBlock _(constattr);
    indent() << "::qi::Signal< ";
    cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
    out() << " >& " << node->name << ";" << std::endl;
    indent() << "::qi::Signal< ";
    cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
    out() << " >& _" << node->name << "() {" << std::endl;
    {
      ScopedIndent _i(_indent);
      indent() << "return " << node->name << ";" << std::endl;
    }
    indent() << "}" << std::endl;
  }
  void visitDecl(PropDeclNode* node) {
    ScopedFormatAttrBlock _(constattr);
    indent() << "::qi::Property< ";
    cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
    out() << " >& " << node->name << ";" << std::endl;
    indent() << "::qi::Property< ";
    cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
    out() << " >& _" << node->name << "() {" << std::endl;
    {
      ScopedIndent _i(_indent);
      indent() << "return " << node->name << ";" << std::endl;
    }
    indent() << "}" << std::endl;
  }

  FormatAttr  virtualAttr;
  FormatAttr  apiAttr;
  std::string apiExport;
};

/// Used for the first pass, to forward-declare interfaces.
class QiLangGenObjectDefFirstPass: public CppTypeFormatter<NodeFormatter<DefaultNodeVisitor>>
{
  void visitStmt(PackageNode* node) override {
    currentNs = splitPkgName(node->name);
    for (unsigned int i = 0; i < currentNs.size(); ++i) {
      indent() << "namespace " << currentNs.at(i) << " {" << std::endl;
    }
  }

  void visitDecl(StructFieldDeclNode* /*node*/) override {}

  void visitDecl(InterfaceDeclNode* node) override
  {
    indent() << "class " << node->name << ";" << std::endl;
    indent() << "using " << node->name << "Ptr = qi::Object<" << node->name << ">;" << std::endl;
    out() << std::endl;
  }

public:
  StringVector currentNs;
};

/// Used for the second pass, to generate the actual code.
class QiLangGenObjectDef: public CppTypeFormatter<>
{
public:
  QiLangGenObjectDef(const PackageManagerPtr& pm, const ParseResultPtr& pr, const StringVector& includes)
    : toclose(0)
    , currentNs()
    , _pm(pm)
    , _pr(pr)
    , _includes(includes)
  {
    _includes.push_back(qiLangToCppInclude(pm->package(pr->package), "api"));

    apiExport = pkgNameToAPI(pr->package);
    //force api export activation by default
    apiAttr.activate();
  }

  FormatAttr  virtualAttr;
  FormatAttr  apiAttr;
  std::string apiExport;

  std::string format(const NodePtrVector &nodes) override
  {
    formatHeader();

    // First pass, includes the namespace declaration on the top of the file
    QiLangGenObjectDefFirstPass firstPassFormatter;
    for (const auto& node: nodes) {
      if (!node)
        throw std::runtime_error("Invalid Node");
      firstPassFormatter.accept(node);
    }
    out() << firstPassFormatter.out().str();

    // Share the namespace information
    currentNs = firstPassFormatter.currentNs;

    // Second pass
    for (const auto& node: nodes) {
      assert(node);
      accept(node);
    }

    formatFooter();
    return this->out().str();
  }

  virtual void doAccept(Node* node) override { node->accept(this); }

  void visitDecl(InterfaceDeclNode* node) override {
    QiLangGenAsyncIface ai(out(), apiExport);
    node->accept(&ai);
    QiLangGenIface si(out(), apiExport);
    node->accept(&si);

    {
      ScopedNamespaceEscaper _e(out(), currentNs);
      out() << "namespace qi {" << std::endl;
      out() << "namespace detail {" << std::endl;
      out() << "  template <>" << std::endl;
      std::string forceProxyInclusionTypeStr = [this, &node]
      {
        std::stringstream ssForceProxyInclusionType;
        ssForceProxyInclusionType << " ForceProxyInclusion< ";
        for (unsigned int i = 0; i < currentNs.size(); ++i) {
          ssForceProxyInclusionType << "::" << currentNs.at(i);
        }
        ssForceProxyInclusionType << "::" << node->name << " >";
        return ssForceProxyInclusionType.str();
      }();
      // define the dummyCall
      out() << "  struct " << apiExport << forceProxyInclusionTypeStr;
      out() << " {" << std::endl;
      out() << "    static bool dummyCall();" << std::endl;
      out() << "  };" << std::endl;
      // call the dummyCall (to force the link of this lib when we will do the #include)
      out() << "  static bool ";
      for (unsigned int i = 0; i < currentNs.size(); ++i) {
        out() << currentNs.at(i);
      }
      out() << node->name << "QiLangDummyVar =" << forceProxyInclusionTypeStr;
      out() << "::dummyCall();" << std::endl;
      out() << "}" << std::endl;
      out() << "}" << std::endl;
    }
  }

  void visitDecl(ParamFieldDeclNode* node) override {
  }

  void visitDecl(FnDeclNode* node) override {
  }

  void visitDecl(SigDeclNode* node) override {
  }
  void visitDecl(PropDeclNode* node) override {
  }

  void visitDecl(StructDeclNode* node) override {
    indent() << "struct " << node->name << " {" << std::endl;
    ScopedFormatAttrBlock _(constattr);
    scoped(node->decls);

    StringVector fields;
    for (unsigned int i = 0; i < node->decls.size(); ++i) {
      if (StructFieldDeclNodePtr field =
          boost::dynamic_pointer_cast<StructFieldDeclNode>(node->decls[i])) {
        for (unsigned int j = 0; j < field->names.size(); ++j)
          fields.push_back(field->names[j]);
      }
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

  void visitDecl(ConstDeclNode* node) override {
    indent() << "const ";
    accept(node->effectiveType());
    out() << " " << node->name;
    if (node->data) {
      out() << " = ";
      accept(node->data);
    }
    out() << ";" << std::endl;
  }

  void visitDecl(StructFieldDeclNode* node) override {
    for (unsigned i = 0; i < node->names.size(); ++i) {
      indent();
      accept(node->effectiveType());
      out() << " " << node->names.at(i);
      if (node->data) {
        out() << " = ";
        accept(node->data);
      }
      out() << ";" << std::endl;
    }
  }

  void visitDecl(EnumDeclNode* node) override {
    indent() << "enum class " << node->name << " {" << std::endl;
    scoped(node->fields);
    indent() << "};" << std::endl << std::endl;
    {
      ScopedNamespaceEscaper _e(out(), currentNs);
      out() << "QI_TYPE_ENUM(";
      for (unsigned int i = 0; i < currentNs.size(); ++i) {
        out() << "::" << currentNs.at(i);
      }
      out() << "::" << node->name << ")" << std::endl << std::endl;
    }
  }
  void visitDecl(EnumFieldDeclNode* node) override {
    if (node->fieldType == EnumFieldType_Const) {
      ConstDeclNode* tnode = static_cast<ConstDeclNode*>(node->node.get());
      indent() << tnode->name << " = ";
      accept(tnode->data);
      out() << "," << std::endl;
      return;
    }
  }
  void visitDecl(TypeDefDeclNode* node) override {
    indent() << "typedef ";
    accept(node->type);
    out() << " " << node->name << ";" << std::endl;
  }

  int toclose;
  StringVector currentNs;
  PackageManagerPtr  _pm;
  const ParseResultPtr& _pr;
  StringVector       _includes;

  void formatHeader() override {
    indent() << "/*" << std::endl;
    indent() << "** qiLang generated file. DO NOT EDIT" << std::endl;
    indent() << "*/" << std::endl;
    indent() << "#pragma once" << std::endl;
    std::string headGuard = filenameToInterfaceHeaderGuard(_pr->package, _pr->filename);
    indent() << "#ifndef " << headGuard << std::endl;
    indent() << "#define " << headGuard << std::endl;
    indent() << std::endl;
    for (unsigned i = 0; i < _includes.size(); ++i) {
      indent() << "#include " << _includes.at(i) << std::endl;
    }
    indent() << std::endl;
  }

  void formatFooter() override {
    for (size_t i = 0; i < currentNs.size(); ++i) {
      out() << "}" << std::endl;
    }
    out() << std::endl;
    indent() << "#endif" << std::endl;
  }

protected:
  void visitStmt(PackageNode*) override {}

  void visitStmt(ImportNode*) override {
  }

  void visitStmt(PropertyDefNode*) override {
    throw std::runtime_error("unimplemented");
  }

  void visitStmt(VarDefNode* node) override {
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
