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
        out() << "virtual public " << node->inherits.at(i) << "Interface";
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

  void visitDecl(ParamFieldDeclNode* node) {
    //useless
  }

  void visitDecl(FnDeclNode* node) {
    indent() << apiAttr(apiExport + " ") << virtualAttr("virtual ");
    acceptTypeExpr(node->effectiveRet());
    out() << " " << node->name << "(";
    cppParamsFormat(this, node->args);
    out() << ")" << virtualAttr(" = 0") << ";" << std::endl;
  }

  void visitDecl(EmitDeclNode* node) {
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
    scopedDecl(node->decls);
    indent() << "};" << std::endl << std::endl;
  }

  void visitDecl(ConstDeclNode* node) {
    indent() << "const ";
    acceptTypeExpr(node->effectiveType());
    out() << " " << node->name;
    if (node->data) {
      out() << " = ";
      acceptData(node->data);
    }
    out() << ";" << std::endl;
  }

  void visitDecl(StructFieldDeclNode* node) {
    for (unsigned i = 0; i < node->names.size(); ++i) {
      indent();
      acceptTypeExpr(node->effectiveType());
      out() << " " << node->names.at(i);
      out() << ";" << std::endl;
    }
  }
  void visitDecl(EnumDeclNode* node) {
    indent() << "enum " << node->name << " {" << std::endl;
    scopedEnumField(node->fields);
    indent() << "};" << std::endl << std::endl;
  }
  void visitDecl(EnumFieldDeclNode* node) {
    if (node->fieldType == EnumFieldType_Const) {
      ConstDeclNode* tnode = static_cast<ConstDeclNode*>(node->node.get());
      indent() << tnode->name << " = ";
      acceptData(tnode->data);
      out() << "," << std::endl;
      return;
    }
    throw std::runtime_error("type in enum not supported in cppi atm");
  }
  void visitDecl(TypeDefDeclNode* node) {
    indent() << "typedef ";
    acceptTypeExpr(node->type);
    out() << " " << node->name << ";" << std::endl;
  }

};

class QiLangGenObjectDef : public FileFormatter
                         , public DeclCppIDLFormatter
                         , public StmtNodeFormatter
{
public:
  QiLangGenObjectDef(const PackageManagerPtr& pm, const ParseResultPtr& pr, const StringVector& includes)
    : toclose(0)
    , _pm(pm)
    , _pr(pr)
    , _includes(includes)
  {
    apiExport = pkgNameToAPI(pr->package);
  }

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

  int toclose;     //number of } to close (namespace)
  PackageManagerPtr  _pm;
  const ParseResultPtr& _pr;
  StringVector       _includes;

  virtual void acceptStmt(const StmtNodePtr &node) { node->accept(this); }

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
    indent();
    acceptTypeExpr(node->effectiveType());
    out() << " " << node->name;
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

std::string genCppObjectInterface(const PackageManagerPtr& pm, const ParseResultPtr& pr) {
  StringVector sv = extractCppIncludeDir(pm, pr, false);
  return QiLangGenObjectDef(pm, pr, sv).format(pr->ast);
}

}
