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
#include "cpptype.hpp"

qiLogCategory("qigen.hppinterface");


namespace qilang {

class DeclCppIDLFormatter : public DeclNodeFormatter, virtual public CppTypeFormatter, virtual public ExprCppFormatter {
public:
  FormatAttr  virtualAttr;
  FormatAttr  apiAttr;
  std::string apiExport;

  DeclCppIDLFormatter() {
    //force api export activation by default
    apiAttr.activate();
  }

  virtual void acceptDecl(const DeclNodePtr& node) { node->accept(this); }

  void visitDecl(InterfaceDeclNode* node) {
    ScopedFormatAttrActivate _(virtualAttr);
    ScopedFormatAttrBlock    _2(apiAttr);

    indent() << "class " << apiExport << " " << node->name << "Interface";
    if (node->inherits.size() > 0) {
      out() << ": ";
      for (unsigned int i = 0; i < node->inherits.size(); ++i) {
        out() << "public " << node->inherits.at(i);
        if (i + 1 != node->inherits.size())
          out() << ", ";
      }
    }
    out() << " {" << std::endl;
    indent() << "public:" << std::endl;
    //add a virtual destructor
    indent() << "  virtual ~" << node->name << "Interface() {}" << std::endl;
    scopedDecl(node->values);
    indent() << "};" << std::endl << std::endl;
    indent() << "typedef qi::Object<" << node->name << "Interface> " << node->name << ";" << std::endl;
  }

  void visitDecl(FnDeclNode* node) {
    indent() << apiAttr(apiExport + " ") << virtualAttr("virtual ");
    if (node->ret) {
      acceptTypeExpr(node->ret);
      out() << " " << node->name << "(";
    } else
      out() << "void " << node->name << "(";

    for (unsigned int i = 0; i < node->args.size(); ++i) {
      consttype(node->args[i]);
      if (i+1 < node->args.size()) {
        out() << ", ";
      }
    }
    out() << ")" << virtualAttr(" = 0") << ";" << std::endl;
  }

  void visitDecl(EmitDeclNode* node) {
    indent() << "qi::Signal< ";
    for (unsigned int i = 0; i < node->args.size(); ++i) {
      acceptTypeExpr(node->args[i]);
      if (i+1 < node->args.size()) {
        out() << ", ";
      }
    }
    out() << " > " << node->name << ";" << std::endl;
  }
  void visitDecl(PropDeclNode* node) {
    indent() << "qi::Property< ";
    for (unsigned int i = 0; i < node->args.size(); ++i) {
      acceptTypeExpr(node->args[i]);
      if (i+1 < node->args.size()) {
        out() << ", ";
      }
    }
    out() << " > " << node->name << ";" << std::endl;
  }

  void visitDecl(StructDeclNode* node) {
    indent() << "struct " << node->name << " {" << std::endl;
    ScopedFormatAttrBlock _(constattr);
    scopedField(node->fields);
    indent() << "};" << std::endl << std::endl;
  }

  void visitDecl(ConstDeclNode* node) {
    indent() << "const ";
    if (node->type) {
      acceptTypeExpr(node->type);
      out() << " " << node->name;
    } else
      out() << "qi::AnyValue " << node->name;
    if (node->data) {
      out() << " = ";
      acceptData(node->data);
    }
    out() << ";" << std::endl;
  }

  void visitDecl(FieldDeclNode* node) {
    if (node->type) {
      indent();
      acceptTypeExpr(node->type);
      out() << " " << node->name;
    }
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
  QiLangGenObjectDef(const PackageManagerPtr& pm, const ParseResult& pr, const StringVector& includes)
    : toclose(0)
    , _pm(pm)
    , _pr(pr)
    , _includes(includes)
  {
    apiExport = pkgNameToAPI(pr.package);
  }

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

  int toclose;     //number of } to close (namespace)
  PackageManagerPtr  _pm;
  const ParseResult& _pr;
  StringVector       _includes;

  virtual void acceptStmt(const StmtNodePtr &node) { node->accept(this); }

  void formatHeader() {
    indent() << "/*" << std::endl;
    indent() << "** qiLang generated file. DO NOT EDIT" << std::endl;
    indent() << "*/" << std::endl;
    indent() << "#pragma once" << std::endl;
    std::string headGuard = filenameToCppHeaderGuard(_pr.package, _pr.filename);
    indent() << "#ifndef " << headGuard << std::endl;
    indent() << "#define " << headGuard << std::endl;
    indent() << std::endl;
    for (unsigned i = 0; i < _includes.size(); ++i) {
      indent() << "#include " << _includes.at(i) << std::endl;
    }
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
    if (node->type) {
      indent() << "";
      acceptTypeExpr(node->type);
      out() << " " << node->name;
    }
    else
      indent() << "qi::AnyValue " << node->name;
    if (node->data) {
      out() << " = ";
      acceptData(node->data);
    }
    out() << ";" << std::endl;
  }
  void visitStmt(CommentNode* node) {
    formatBlock(out(), node->comments, "// ", _indent);
  }

};

std::string genCppObjectInterface(const PackageManagerPtr& pm, const ParseResult& nodes) {
  StringVector sv = extractCppIncludeDir(pm, nodes, false);
  return QiLangGenObjectDef(pm, nodes, sv).format(nodes.ast);
}

}
