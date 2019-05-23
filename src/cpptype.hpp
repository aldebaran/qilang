/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Cedric GESTES
*/
#ifndef CPPTYPE_HPP
#define CPPTYPE_HPP

#include <string>
#include <vector>
#include <qilang/node.hpp>
#include <qilang/packagemanager.hpp>
#include "formatter_p.hpp"

namespace qilang {

  /** Cpp Type Formatter.
   *
   *  use consttype(type) to ask for a const type.
   *  if noconstref == 0 it will be honored.
   *
   *  noconst increment noconstref to avoid constref even when they are asked.
   *  Think string in a vector.
   */
  template <typename T = NodeFormatter<> >
  class CppTypeFormatter: public T {
  public:
    //should we add const if possible? (for function params)
    //contextual disable const ref.  (const std::vector<std::string>&)
    FormatAttr constattr;

    explicit CppTypeFormatter();
    explicit CppTypeFormatter(std::stringstream& ss, int indent = 0);

    virtual void doAccept(Node* node) { node->accept(this); }

    void unconstify(TypeExprNodePtr node);

    //ask for a const ref expression.
    void constify(const TypeExprNodePtr& node);

    void visitTypeExpr(BuiltinTypeExprNode* node);
    void visitTypeExpr(CustomTypeExprNode* node);
    void visitTypeExpr(ListTypeExprNode* node);
    void visitTypeExpr(MapTypeExprNode* node);
    void visitTypeExpr(TupleTypeExprNode* node);
    void visitTypeExpr(OptionalTypeExprNode* node);
    void visitTypeExpr(VarArgTypeExprNode* node);
    void visitTypeExpr(KeywordArgTypeExprNode* node);

    void visitData(BoolLiteralNode *node);
    void visitData(IntLiteralNode *node);
    void visitData(FloatLiteralNode *node);
    void visitData(StringLiteralNode *node);
    void visitData(TupleLiteralNode* node);
    void visitData(ListLiteralNode* node);
    void visitData(DictLiteralNode* node);

    void visitExpr(BinaryOpExprNode *node);
    void visitExpr(UnaryOpExprNode *node);
    void visitExpr(VarExprNode *node);
    void visitExpr(LiteralExprNode* node);
    void visitExpr(CallExprNode* node);
  };


  // if self == true then and include on self is returned
  StringVector extractCppIncludeDir(const PackageManagerPtr& pm, const ParseResultPtr& nodes, bool self);

  //std::string typeToCpp(TypeExprNode* type, bool constref=true);
  //pkgName to include dir
  std::string pkgNameToDir(const std::string& name);

  //pkgName to *_API var
  std::string pkgNameToAPI(const std::string& name);

  std::vector<std::string> splitPkgName(const std::string& name);
  std::string formatNs(const std::string& package);

  std::string filenameToHeaderGuardBase(const std::string& pkgName, const std::string& filename);
  std::string filenameToInterfaceHeaderGuard(const std::string& pkgName, const std::string& filename);

  void formatBlock(std::ostream& os, const std::string& name, const std::string& sep, int indent);

  enum CppParamsFormat {
    CppParamsFormat_Normal,
    CppParamsFormat_TypeOnly,
    CppParamsFormat_NameOnly,
  };
  template <typename T>
  void cppParamsFormat(CppTypeFormatter<T>* typeformat, ParamFieldDeclNodePtrVector node, CppParamsFormat cfpt = CppParamsFormat_Normal);

}

#include <cpptype.hxx>

#endif
