/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Cedric GESTES
*/

#include <string>
#include <vector>
#include <qilang/node.hpp>
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
  class CppTypeFormatter: public TypeExprNodeFormatter {
  public:
    explicit CppTypeFormatter();

    virtual void acceptTypeExpr(const TypeExprNodePtr& node);

    const std::string& noconst(TypeExprNodePtr node);
    const std::string& addconst();
    const std::string& addref();

    //ask for a const ref expression.
    const std::string& consttype(const TypeExprNodePtr& node);

    void visitTypeExpr(SimpleTypeExprNode* node);
    void visitTypeExpr(ListTypeExprNode* node);
    void visitTypeExpr(MapTypeExprNode* node);
    void visitTypeExpr(TupleTypeExprNode* node);

    int addconstref;   //should we add const if possible? (for function params)
    int noconstref;    //contextual disable const ref.  (const std::vector<std::string>&)
  };

  class DataCppFormatter : public ConstDataNodeFormatter {
  public:
    virtual void acceptData(const ConstDataNodePtr& node);
    void visitData(BoolConstDataNode *node);
    void visitData(IntConstDataNode *node);
    void visitData(FloatConstDataNode *node);
    void visitData(StringConstDataNode *node);
    void visitData(TupleConstDataNode* node);
    void visitData(ListConstDataNode* node);
    void visitData(DictConstDataNode* node);
  };

  class ExprCppFormatter : public ExprNodeFormatter, virtual public DataCppFormatter {
  public:
    virtual void acceptExpr(const ExprNodePtr& node) { node->accept(this); }

    void visitExpr(BinaryOpExprNode *node);
    void visitExpr(UnaryOpExprNode *node);
    void visitExpr(VarExprNode *node);
    void visitExpr(ConstDataExprNode* node);
  };


  //std::string typeToCpp(TypeExprNode* type, bool constref=true);
  std::vector<std::string> splitPkgName(const std::string& name);

}
