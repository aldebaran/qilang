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

qiLogCategory("qigen.hppgmock");

namespace qilang
{
/**
 * Generates the content of a registration function for a given object type.
 */
struct QiLangGenRegisterObject: CppTypeFormatter<NodeFormatter<DefaultNodeVisitor>>
{
  QiLangGenRegisterObject(std::stringstream& ss, int indent)
    : CppTypeFormatter<NodeFormatter<DefaultNodeVisitor>>(ss, indent)
  {
  }

  void cppParamsFormatUnconstifiedTypes(ParamFieldDeclNodePtrVector& params)
  {
    // for registration, we want an unconstified declaration.
    bool first = true;
    for (auto& paramNode: params) {
      if (!first)
        out() << ", ";
      accept(paramNode->effectiveType());
      first = false;
    }
  }

  void visitDecl(InterfaceDeclNode* node) override
  {
    for (auto& child: node->values)
      accept(child);
    indent() << "typeBuilder.registerType();\n";
  }

  void visitDecl(FnDeclNode* node) override
  {
    indent() << "typeBuilder.advertiseMethod(\"" << node->name << "\", static_cast<::qi::Future<";
    accept(node->effectiveRet());
    out() << ">(T::*)(";
    cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
    out() << ")>(&T::" << node->name << "));\n";
  }

  void visitDecl(SigDeclNode* node) override
  {
    indent() << "typeBuilder.advertiseSignal(\"" << node->name << "\", static_cast<::qi::Signal<";
    cppParamsFormatUnconstifiedTypes(node->args);
    out() << "> T::*>(&T::" << node->name << "));\n";
  }

  void visitDecl(PropDeclNode* node) override
  {
    indent() << "typeBuilder.advertiseProperty(\"" << node->name << "\", static_cast<::qi::Property<";
    cppParamsFormatUnconstifiedTypes(node->args);
    out() << "> T::*>(&T::" << node->name << "));\n";
  }
};

/**
 * Generates a C++ header defining GMock classes complying with the associated interface,
 * for each interface declared in the QiLang file.
 */
class QiLangGenGMock: public CppTypeFormatter<NodeFormatter<DefaultNodeVisitor>>
{
public:
  QiLangGenGMock(const PackageManagerPtr& pm, const ParseResultPtr& pr)
    : _currentNs()
    , _pm(pm)
    , _pr(pr)
    , _headerGuard("_QILANG_GEN_GMOCK_" + filenameToHeaderGuardBase(_pr->package, _pr->filename) + "_HPP")
  {
  }

  std::string format(const NodePtrVector &nodes) override
  {
    formatHeader();
    for (const auto& node: nodes)
    {
      assert(node);
      accept(node);
    }
    formatFooter();
    return this->out().str();
  }

  void formatHeader() override {
    indent() << "// QiLang-generated file. DO NOT EDIT.\n";
    indent() << "#pragma once\n";
    indent() << "#ifndef " << _headerGuard << "\n";
    indent() << "#define " << _headerGuard << "\n";
    indent() << "\n";
    indent() << "#include <gmock/gmock.h>\n";
    indent() << "#include " << qiLangToCppInclude(_pm->package(_pr->package), _pr->filename) << "\n";
    indent() << "\n";

    // Namespaces are opened when encountering package node statements,
    // this explains that the code of formatters are usually assymmetric.
  }

  void formatFooter() override {
    for (const auto& ns: _currentNs) {
      indent() << "} // ends namespace " << ns << "\n";
    }
    out() << "\n";
    indent() << "#endif // " << _headerGuard << "\n";
  }

  void visitStmt(PackageNode* node) override {
    _currentNs = splitPkgName(node->name);
    _currentNs.push_back("gmock");

    for (const auto& ns: _currentNs) {
      indent() << "namespace " << ns << "\n";
      indent() << "{\n";
    } // scopes will be closed in the footer
  }

  void visitDecl(InterfaceDeclNode* node) override
  {
    // Preamble: make a function, in a "detail" namespace, that can register any type
    // matching the interface. TODO: in C++14, it should be replaced by a lambda accepting
    // auto arguments, defined directly in the constructor.
    const std::string registrationFunctionName = "register" + node->name + "InTypeBuilder";
    indent() << "namespace detail\n";
    indent() << "{\n";
    indent() << "template <typename T>\n";
    indent() << "void " << registrationFunctionName
             << "(::qi::ObjectTypeBuilder<T>&& typeBuilder)\n";
    indent() << "{\n";
    {
      ScopedIndent registrationIndent(_indent);
      QiLangGenRegisterObject registrationGen{out(), _indent};
      node->accept(&registrationGen); // generates the call for "registerType"
    }
    indent() << "}\n";
    indent() << "} // ends namespace detail\n" << "\n";

    // Forward declaring types before defining the GMock type,
    // so that they can be registered automagically in constructor.
    const std::string mockClassName = node->name + "GMock";
    const std::string niceMockClassName = node->name + "NiceGMock";
    const std::string strictMockClassName = node->name + "StrictGMock";
    indent() << "struct " << mockClassName << ";\n";
    indent() << "using " << niceMockClassName
             << " = ::testing::NiceMock<" << mockClassName << ">;\n";
    indent() << "using " << strictMockClassName
             << " = ::testing::StrictMock<" << mockClassName << ">;\n";
    indent() << "\n";

    indent() << "struct " << mockClassName << "\n";
    indent() << "{\n";
    {
      // Magic constructor that registers the GMock-related types.
      ScopedIndent classIndent(_indent);
      indent() << mockClassName << "()\n";
      indent() << "{\n";
      {
        ScopedIndent constructorIndent(_indent);
        indent() << "static bool _typeRegistered_ = []\n";
        indent() << "{\n";
        {
          ScopedIndent registrationIndent(_indent);
          auto generateTypeRegistrationCall = [&](const std::string& className)
          {
            indent() << "detail::" << registrationFunctionName
                     << "(::qi::ObjectTypeBuilder<" << className << ">{});\n";
          };
          generateTypeRegistrationCall(mockClassName);
          generateTypeRegistrationCall(niceMockClassName);
          generateTypeRegistrationCall(strictMockClassName);
          indent() << "return true;\n";
        } // registration
        indent() << "}();\n";
        indent() << "QI_UNUSED(_typeRegistered_);\n";
      } // construction
      indent() << "}\n" << "\n";

      for (auto& childNode: node->values)
        this->accept(childNode);
    } // class
    indent() << "};\n" << "\n";
  }

  void visitDecl(FnDeclNode* node) override
  {
    // always expect an asynchronous implementation for the mockup
    indent() << "MOCK_METHOD" << node->args.size() << "(" << node->name << ", ::qi::Future<";
    NodeFormatter::accept(node->effectiveRet());
    out() << ">(";
    cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
    out() << "));\n";
  }

  void visitDecl(SigDeclNode* node) override {
    ScopedFormatAttrBlock _(constattr);
    indent() << "qi::Signal<";
    cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
    out() << "> " << node->name << ";\n";
  }

  void visitDecl(PropDeclNode* node) override {
    ScopedFormatAttrBlock _(constattr);
    indent() << "qi::Property<";
    cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
    out() << "> " << node->name << ";\n";
  }

  void visitDecl(StructFieldDeclNode*) override {
    // ignore structs properly, default implementation produces some output otherwise
  }

private:
  StringVector _currentNs;
  PackageManagerPtr  _pm;
  const ParseResultPtr& _pr;
  const std::string _headerGuard;
};


std::string genCppGMock(const PackageManagerPtr& pm, const ParseResultPtr& pr)
{
  return QiLangGenGMock{pm, pr}.format(pr->ast);
}

} // qilang
