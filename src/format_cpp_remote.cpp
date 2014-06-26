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

qiLogCategory("qigen.cppremote");

namespace qilang {

  class CppRemoteGenFormatter : public DeclNodeFormatter, virtual public CppTypeFormatter, virtual public ExprCppFormatter {
  public:
    CppRemoteGenFormatter()
    {}

    FormatAttr methodAttr;

    virtual void acceptDecl(const DeclNodePtr& node) { node->accept(this); }

    void visitDecl(InterfaceDeclNode* node) {
      indent() << "class " << node->name + "Remote" << ": public " << node->name + "Interface";
      //there is some inherits, so proxy is already inherited by the parent.
      if (node->inherits.size() == 0) {
        out() << ", public qi::Proxy {" << std::endl;
      } else {
        for (unsigned i = 0; i < node->inherits.size(); ++i) {
          out() << ", public " << node->inherits.at(i) << "Remote";
        }
        out() << " {" << std::endl;
      }

      indent() << "public:" << std::endl;

      {
        ScopedIndent _(_indent);
        ScopedFormatAttrActivate _2(methodAttr);

        indent() << node->name + "Remote(const qi::AnyObject& ao)" << std::endl;
        if (node->inherits.size() > 0)
        {
          indent() << "  : " << node->inherits[0] << "Remote(ao)" << std::endl;
          for (unsigned i = 1; i < node->inherits.size(); ++i) {
            indent() << ", " << node->inherits[i] << "Remote(ao)" << std::endl;
          }

        } else {
          indent() << "  : qi::Proxy(ao)" << std::endl;
        }
        indent() << "{}" << std::endl;



        for (unsigned int i = 0; i < node->values.size(); ++i) {
          acceptDecl(node->values.at(i));
        }
      }
      out() << std::endl;
      indent() << "};" << std::endl;
      indent() << "QI_REGISTER_PROXY_INTERFACE(" << node->name + "Remote, " << node->name << "Interface);" << std::endl;
      indent() << std::endl;
    }

    void walkParams(const ParamFieldDeclNodePtrVector& params) {
      for (unsigned i = 0; i < params.size(); ++i) {
        acceptDecl(params.at(i));
        if (i + 1 < params.size())
          out() << ", ";
      }
    }

    void visitDecl(ParamFieldDeclNode* node) {
      constify(node->type);
      out() << " " << node->names.at(0);
      qiLogWarning() << "param visitor not fully impl";
    }

    void visitDecl(FnDeclNode* node) {
      if (!methodAttr.isActive())
        return;
      indent();
      acceptTypeExpr(node->effectiveRet());
      out() << " " << node->name << "(";
      cppParamsFormat(this, node->args);
      out() << ") {" << std::endl;
      {
        ScopedIndent _(_indent);
        if (!node->hasNoReturn())
        {
          indent() << "return _obj.call< ";
          acceptTypeExpr(node->ret);
          out() << " >(";
        }
        else
          indent() << "_obj.call<void>(";
        out() << "\"" << node->name << "\"";
        if (node->args.size() != 0)
          out() << ", ";
        cppParamsFormat(this, node->args, CppParamsFormat_NameOnly);
        out() << ");" << std::endl;
      }
      indent() << "}" << std::endl;
    }

    void visitDecl(EmitDeclNode* node) {
      qiLogError() << "EmitDeclNode not implemented";
    }
    void visitDecl(PropDeclNode* node) {
      qiLogError() << "PropDeclNode not implemented";
    }
    void visitDecl(StructDeclNode* node) {
      qiLogError() << "StructDeclNode not implemented";
    }
    void visitDecl(ConstDeclNode* node) {
      qiLogError() << "FieldDeclNode not implemented";
    }
    void visitDecl(StructFieldDeclNode* node) {
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

  };

//Generate Type Registration Information
class CppRemoteQiLangGen: public FileFormatter,
                          public StmtNodeFormatter,
                          public CppRemoteGenFormatter
{
public:
  CppRemoteQiLangGen(const PackageManagerPtr& pm, const StringVector& includes)
    : toclose(0)
    , _includes(includes)
  {}

  int toclose;
  StringVector _includes;

  virtual void acceptStmt(const StmtNodePtr& node) { node->accept(this); }

  virtual void accept(const NodePtr& node) {
    switch (node->kind()) {
    case NodeKind_Literal:
      acceptData(boost::dynamic_pointer_cast<LiteralNode>(node));
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
  void visitStmt(CommentNode* node) {
    formatBlock(out(), node->comments, "// ", _indent);
  }

};

std::string genCppObjectRemote(const PackageManagerPtr& pm, const ParseResultPtr& pr) {
  StringVector sv = extractCppIncludeDir(pm, pr, true);
  return CppRemoteQiLangGen(pm, sv).format(pr->ast);
}



}

