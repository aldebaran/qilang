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

  class LocalCppGenFormatter : public DeclNodeFormatter, virtual public CppTypeFormatter, virtual public ExprCppFormatter {
  public:
    LocalCppGenFormatter()
    {}

    FormatAttr methodAttr;

    virtual void acceptDecl(const DeclNodePtr& node) { node->accept(this); }

    void visitDecl(InterfaceDeclNode* node) {
      indent() << "template<class T>" << std::endl;
      indent() << "class " << node->name + "Remote" << ": public " << node->name + "Interface, public T {" << std::endl;
      indent() << "public:" << std::endl;
      {
        ScopedIndent _(_indent);
        ScopedFormatAttrActivate _2(methodAttr);
        for (unsigned int i = 0; i < node->values.size(); ++i) {
          acceptDecl(node->values.at(i));
        }
      }
      out() << std::endl;
      indent() << "protected:" << std::endl;
      indent() << "  qi::AnyObject _object;" << std::endl;
      indent() << "};" << std::endl;
      indent() << std::endl;
    }

    void visitDecl(FnDeclNode* node) {
      if (node->ret) {
        indent() << "";
        acceptTypeExpr(node->ret);
      } else {
        indent() << "void";
      }
      out() << " " << node->name << "(";

      for (unsigned int i = 0; i < node->args.size(); ++i) {
        consttype(node->args[i]);
        out() << " arg" << i;
        if (i+1 < node->args.size()) {
          out() << ", ";
        }
      }
      out() << ") {" << std::endl;
      {
        ScopedIndent _(_indent);
        indent() << "return qi::wrapFuture(), T::" << node->name << "(";
        for (unsigned int i = 0; i < node->args.size(); ++i) {
          out() << "arg" << i;
          if (i+1 < node->args.size()) {
            out() << ", ";
          }
        }
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
    void visitDecl(FieldDeclNode* node) {
      qiLogError() << "FieldDeclNode not implemented";
    }
  };

//Generate Type Registration Information
class QiLangGenObjectLocal  : public FileFormatter,
                              public StmtNodeFormatter,
                              public LocalCppGenFormatter
{
public:
  QiLangGenObjectLocal(const PackageManagerPtr& pm, const StringVector& includes)
    : toclose(0)
    , _includes(includes)
  {}

  int toclose;
  StringVector _includes;

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
  void visitStmt(CommentNode* node) {
    formatBlock(out(), node->comments, "// ", _indent);
  }

};

std::string genCppObjectLocal(const PackageManagerPtr& pm, const ParseResult& nodes) {
  StringVector sv = extractCppIncludeDir(pm, nodes, true);
  return QiLangGenObjectLocal(pm, sv).format(nodes.ast);
}



}
