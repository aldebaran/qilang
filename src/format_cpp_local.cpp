/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <qi/log.hpp>
#include <qilang/node.hpp>
#include <qilang/formatter.hpp>
#include <qilang/visitor.hpp>
#include <qilang/docparser.hpp>
#include <qi/os.hpp>
#include "formatter_p.hpp"
#include "cpptype.hpp"

qiLogCategory("qigen.cpplocal");

namespace qilang {

  static const char* ImplTypeName = "impl__";

  class QiLangGenObjectLocalAsync: public CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >
  {
  public:
    QiLangGenObjectLocalAsync(std::stringstream& ss, int indent)
      : CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >(ss, indent)
    {}

    std::string selfName;

    void visitDecl(InterfaceDeclNode* node) {
      selfName = node->name;
      indent() << "template <typename ImplPtr>" << std::endl;
      indent() << "class " << node->name << "LocalAsync : public " << node->name << "Async" << std::endl;
      indent() << "{" << std::endl;
      indent() << "public:" << std::endl;
      {
        ScopedIndent _(_indent);
        indent() << "explicit " << node->name << "LocalAsync(ImplPtr impl)" << std::endl;
        indent() << "  : _p(std::move(impl))" << std::endl;
        indent() << "{}" << std::endl;

        for (unsigned int i = 0; i < node->values.size(); ++i) {
          accept(node->values.at(i));
        }
      }

      out() << std::endl;
      indent() << "private:" << std::endl;
      {
        ScopedIndent _(_indent);
        indent() << "ImplPtr _p;" << std::endl;
        indent() << "using ImplType = typename ImplPtr::element_type;" << std::endl;
      }

      indent() << "};" << std::endl;
      indent() << std::endl;
      selfName.clear();
    }

    void visitDecl(FnDeclNode* node) {
      indent() << "::qi::Future<";
      accept(node->effectiveRet());
      out() << "> " << node->name << "(";
      cppParamsFormat(this, node->args);
      out() << ")" << std::endl;
      indent() << "{" << std::endl;
      {
        ScopedIndent _(_indent);

        const auto outputArgs = [&](CppParamsFormat format, bool withComma = true) {
          if (!node->args.empty()) {
            if (withComma)
              out() << ", ";
            cppParamsFormat(this, node->args, format);
          }
        };

        // Use a lambda for easier type deduction.
        // We want to access `this` only once access is checked as safe (see below)
        // so we take it as parameter passed by the code doing the check.
        // Consequently nothing needs to be captured, so the lambda can be static.
        indent() << "static auto f = [](const ImplPtr& self";
        outputArgs(CppParamsFormat_Normal);
        out() << "){ QI_ASSERT_NOT_NULL(self); return self->" << node->name << "(";
        outputArgs(CppParamsFormat_NameOnly, false);
        out() << "); };" << std::endl;

        // Deduce the return type of the implementation
        indent() << "using ReturnType = decltype(f(_p";
        outputArgs(CppParamsFormat_NameOnly);
        out() << "));" << std::endl;

        // Use `qilang::detail::safeMemberAsync()` for asynchronicity, tryUnwrap to fallback on a simple future,
        // whatever the return type of the member function of the implementation.
        // @see `qilang::detail::safeMemberAsync()` for details.
        indent() << "return qi::detail::tryUnwrap(qilang::detail::safeMemberAsync<ReturnType, ImplType>(f, _p";
        outputArgs(CppParamsFormat_NameOnly);
        out() << "));" << std::endl;
      }
      indent() << "}" << std::endl << std::endl; // let an empty line after function definition
    }

    void visitDecl(SigDeclNode*) {}
    void visitDecl(PropDeclNode*) {}
  };

  class QiLangGenObjectLocalSync: public CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >
  {
  public:
    QiLangGenObjectLocalSync(std::stringstream& ss, int indent)
      : CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >(ss, indent)
    {}

    void visitDecl(InterfaceDeclNode* node) {
      indent() << "template <typename ImplPtr>" << std::endl;
      indent() << "class " << node->name << "LocalSync : public " << node->name << ", public qi::Proxy" << std::endl;
      indent() << "{" << std::endl;
      indent() << "public:" << std::endl;
      {
        ScopedIndent _(_indent);
        indent() << "explicit " << node->name << "LocalSync(ImplPtr impl)" << std::endl;
        indent() << "  : " << node->name << "(";
        {
          bool first = true;
          for (unsigned int i = 0; i < node->values.size(); ++i) {
            if (node->values.at(i)->type() == NodeType_SigDecl) {
              if (first)
                first = false;
              else
                out() << ", ";
              out() << "impl->" << boost::static_pointer_cast<SigDeclNode>(node->values.at(i))->name;
            }
            if (node->values.at(i)->type() == NodeType_PropDecl) {
              if (first)
                first = false;
              else
                out() << ", ";
              out() << "impl->" << boost::static_pointer_cast<PropDeclNode>(node->values.at(i))->name;
            }
          }
        }
        out() << ")" << std::endl;
        indent() << "  , _async(impl)" << std::endl;
        indent() << "{}" << std::endl << std::endl;

        for (unsigned int i = 0; i < node->values.size(); ++i) {
          accept(node->values.at(i));
        }

        out() << std::endl;
        indent() << node->name << "Async& async()" << std::endl;
        indent() << "{" << std::endl;
        indent() << "  return *static_cast<" << node->name << "Async*>(&_async);" << std::endl;
        indent() << "}" << std::endl;
      }

      out() << std::endl;
      indent() << "private:" << std::endl;
      {
        ScopedIndent _(_indent);
        indent() << node->name << "LocalAsync<ImplPtr> _async;" << std::endl;
      }

      indent() << "};" << std::endl;
      indent() << std::endl;
    }

    void visitDecl(FnDeclNode* node) {
      indent();
      accept(node->effectiveRet());
      out() << " " << node->name << "(";
      cppParamsFormat(this, node->args);
      out() << ")" << std::endl;
      indent() << "{" << std::endl;
      {
        ScopedIndent _(_indent);
        if (node->ret) {
          indent() << "return ";
        }
        else {
          indent();
        }
        out() << "_async." << node->name << "(";
        cppParamsFormat(this, node->args, CppParamsFormat_NameOnly);
        out() << ").value();" << std::endl;
      }
      indent() << "}" << std::endl;
    }
    void visitDecl(SigDeclNode* node) {}
    void visitDecl(PropDeclNode* node) {}
  };

  class QiLangGenObjectBind: public CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >
  {
  public:
    QiLangGenObjectBind(std::stringstream& ss, const StringVector& ns)
      : CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >(ss)
    {
      BOOST_FOREACH(const std::string& nspart, ns) {
        _ns += "::" + nspart;
      }
    }

    std::string _ns;
    std::string _curName;
    std::string _fullName;
    FormatAttr _methodBounceAttr;

    void visitDecl(InterfaceDeclNode* node) {
      _fullName = _ns + "::" + node->name;
      _curName = node->name;


      indent() << "#define REGISTER_" << boost::to_upper_copy<std::string>(node->name) << "(" << ImplTypeName << ") \\" << std::endl;
      indent() << "static_assert(\\" << std::endl;
      {
        ScopedIndent moreIndent(_indent, 4);
        indent() << "qi::detail::InterfaceImplTraits<" << _fullName << ">::Defined::value,\\" << std::endl;
        indent() << "\"Missing QI_REGISTER_IMPLEMENTATION_H(" << _fullName
                 << ", \" #impl__ \") in the header of the implementation\");\\" << std::endl;
      }
      indent() << "QI_REGISTER_IMPLEMENTATION(" << _fullName << ", qi::detail::InterfaceImplTraits< " << _fullName
        << " >::SyncType) \\" << std::endl;
      {
        ScopedFormatAttrActivate _2(_methodBounceAttr);
        for (unsigned int i = 0; i < node->values.size(); ++i) {
          accept(node->values.at(i));
        }
      }
      indent() << "static int initType" << node->name << "() { \\" << std::endl;
      {
        ScopedIndent _(_indent);
        indent() << "qi::ObjectTypeBuilder< " << _fullName << " > builder; \\" << std::endl;
        for (unsigned int i = 0; i < node->inherits.size(); ++i) {
          indent() << "builder.inherits< " << node->inherits.at(i) << " >(); \\" << std::endl;
        }
        for (unsigned int i = 0; i < node->values.size(); ++i) {
          accept(node->values.at(i));
        }
        indent() << "builder.registerType(); \\" << std::endl;
        indent() << "return 42; \\" << std::endl;
      }
      indent() << "} \\" << std::endl;
      indent() << "static int myinittype" << node->name << " = initType" << node->name << "();" << std::endl;
      indent() << std::endl;

      _fullName.clear();
      _curName.clear();
    }

    void visitDecl(FnDeclNode* node) {
      if (_methodBounceAttr.isActive()) {
        indent() << "static ::qi::Future< ";
        accept(node->effectiveRet());
        out() << " > " << _curName << node->name << "(" << _fullName << "* obj";
        if (!node->args.empty()) {
          out() << ", ";
          cppParamsFormat(this, node->args);
        }
        out() << ") { \\" << std::endl;
        {
          ScopedIndent _(_indent);
          indent() << "return static_cast<qi::detail::InterfaceImplTraits< " << _fullName
            << " >::SyncType*>(obj)->async()." << node->name << "(";
          cppParamsFormat(this, node->args, CppParamsFormat_NameOnly);
          out() << "); \\" << std::endl;
        }
        indent() << "} \\" << std::endl;
      }
      else {
        indent() << "{ \\" << std::endl;
        {
          ScopedIndent _(_indent);
          Doc doc = parseDoc(node->comment());
          indent() << "qi::MetaMethodBuilder mmb; \\" << std::endl;
          indent() << "mmb.setName(\"" << node->name << "\"); \\" << std::endl;
          BOOST_FOREACH(Doc::Parameters::value_type it, doc.parameters) {
            indent() << "mmb.appendParameter(\"" << it.first << "\", \"" << it.second << "\"); \\" << std::endl;
          }
          if (doc.return_)
            indent() << "mmb.setReturnDescription(\"" << *doc.return_ << "\"); \\" << std::endl;
          if (doc.description)
            indent() << "mmb.setDescription(\"" << *doc.description << "\"); \\" << std::endl;
          indent() << "const auto callType = std::is_base_of<qi::Actor, " << ImplTypeName << " >::value ?"
            " qi::MetaCallType_Direct : qi::MetaCallType_Auto; \\" << std::endl;
          indent() << "builder.advertiseMethod(mmb, static_cast<::qi::Future< ";
          accept(node->effectiveRet());
          out() << " > (*)(" << _fullName << "*";
          if (!node->args.empty())
          {
            out() << ", ";
            cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
          }
          out() << ")>(&" << _curName << node->name;
          out() << "), callType); \\" << std::endl;
        }
        indent() << "} \\" << std::endl;
      }
    }

    void visitDecl(SigDeclNode* node) {
      if (!_methodBounceAttr.isActive()) {
        indent() << "builder.advertiseSignal(\"" << node->name << "\", &" << _fullName << "::_" << node->name
          << "); \\" << std::endl;
      }
    }

    void visitDecl(PropDeclNode* node) {
      if (!_methodBounceAttr.isActive()) {
        indent() << "builder.advertiseProperty(\"" << node->name << "\", &" << _fullName << "::_" << node->name
          << "); \\" << std::endl;
      }
    }
  };


  class QiLangGenObjects: public NodeFormatter<DefaultNodeVisitor>
  {
  public:
    int toclose;
    StringVector _includes;
    StringVector _ns;

    QiLangGenObjects(StringVector includes, std::string packageName, std::string fileName)
      : toclose(0)
      , _includes(includes)
      , _packageName(std::move(packageName))
      , _fileName(fileName)
    {
      _includes.push_back("<boost/smart_ptr/enable_shared_from_raw.hpp>");
      _includes.push_back("<qi/assert.hpp>");
    }

    void visitDecl(InterfaceDeclNode* node) {
      QiLangGenObjectLocalAsync locasync(out(), _indent);
      node->accept(&locasync);
      QiLangGenObjectLocalSync locsync(out(), _indent);
      node->accept(&locsync);

      closeNamespace();

      QiLangGenObjectBind bind(out(), _ns);
      node->accept(&bind);

      openNamespace();
    }

    void visitStmt(PackageNode* node) {
      _ns = splitPkgName(node->name);
      openNamespace();
    }

    void openNamespace() {
      for (unsigned int i = 0; i < _ns.size(); ++i) {
        toclose++;
        indent() << "namespace " << _ns.at(i) << " {" << std::endl;
      }
      out() << std::endl;
    }

    void closeNamespace() {
      for (size_t i = 0; i < _ns.size(); ++i) {
        out() << "}" << std::endl;
      }
    }

    void formatHeader() {
      indent() << "/*" << std::endl;
      indent() << "** qiLang generated file. DO NOT EDIT" << std::endl;
      indent() << "*/" << std::endl;
      auto headerGuard = filenameToInterfaceHeaderGuard(_packageName, _fileName) + "_P";
      indent() << "#ifndef " << headerGuard << std::endl;
      indent() << "#define " << headerGuard << std::endl;

      indent() << "#include <qi/future.hpp>" << std::endl;
      for (unsigned i = 0; i < _includes.size(); ++i) {
        indent() << "#include " << _includes.at(i) << std::endl;
      }
      indent() << std::endl;

      // We need to inject manually the content of common-gen.hpp because
      // targets using qicc do not necessarily depend on libqilang.
      // Instead of copying the content of the common code in each fonction
      // (making it untestable) we prefer to inject it at generation-time
      // in each generated file.
      // The code will be protected by header guards.
      const char* commonCode =
      #include <qilang/detail/gencodeutility.txt>
      ;
      out() << commonCode;
      indent() << std::endl;
    }

    void formatFooter() {
      closeNamespace();
      auto headerGuard = filenameToInterfaceHeaderGuard(_packageName, _fileName);
      indent() << "#endif // " << headerGuard << std::endl;
    }

  private:
    const std::string _packageName;
    const std::string _fileName;
  };

std::string genCppObjectLocal(const PackageManagerPtr& pm, const ParseResultPtr& pr) {
  StringVector sv = extractCppIncludeDir(pm, pr, true);
  return QiLangGenObjects(sv, pr->package, pr->filename).format(pr->ast);
}
}
