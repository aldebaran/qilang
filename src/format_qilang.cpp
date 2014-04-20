/*
** Author(s):
**  - Cedric GESTES  <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <sstream>
#include <stdexcept>
#include <qilang/node.hpp>
#include <qilang/formatter.hpp>
#include "formatter_p.hpp"

namespace qilang {

  class QiLangConstDataFormatter : public ConstDataNodeFormatter {
  public:
    virtual void accept(const ConstDataNodePtr& node) { node->accept(this); }

    const std::string &list(ConstDataNodePtrVector pv) {
      static const std::string ret;
      for (int i = 0; i < pv.size(); ++i) {
        cdata(pv.at(i));
        if (i + 1 < pv.size())
          out() << ", ";
      }
      return ret;
    }

    const std::string &dict(ConstDataNodePtrPairVector pv) {
      static const std::string ret;
      for (int i = 0; i < pv.size(); ++i) {
        out() << cdata(pv.at(i).first) << " : " << cdata(pv.at(i).second);
        if (i + 1 < pv.size())
          out() << ", ";
      }
      return ret;
    }

    void visit(BoolConstDataNode *node) {
      if (node->value)
        out() << "true";
      else
        out() << "false";
    }
    void visit(IntConstDataNode *node) {
      out() << node->value;
    }
    void visit(FloatConstDataNode *node) {
      out() << node->value;
    }
    void visit(StringConstDataNode *node) {
      out() << node->value;
    }
    void visit(ListConstDataNode* node) {
      out() << "[ " << list(node->values) << " ]";
    }
    void visit(TupleConstDataNode* node) {
      out() << "( " << list(node->values) << " )";
    }
    void visit(DictConstDataNode* node) {
      out() << "{ " << dict(node->values) << " }";
    }
  };

  class QiLangFormatter : public StmtNodeFormatter,
                          public DeclNodeFormatter,
                          public TypeExprNodeFormatter,
                          public ExprNodeFormatter,
                          public QiLangConstDataFormatter {
  protected:
    virtual void accept(const StmtNodePtr& node)      { node->accept((StmtNodeVisitor*)this); }
    virtual void accept(const DeclNodePtr& node)      { node->accept((DeclNodeVisitor*)this); }
    virtual void accept(const ExprNodePtr& node)      { node->accept((ExprNodeVisitor*)this); }
    virtual void accept(const TypeExprNodePtr& node)  { node->accept((TypeExprNodeVisitor*)this); }

    void visit(PackageNode* node) {
      indent() << "package " << node->name << std::endl;
    }

    void visit(ImportNode* node) {
      if (node->imported.size() == 0)
        indent() << "import " << node->name;
      else {
        indent() << "from " << node->name << " import ";
        for (unsigned int i = 0; i < node->imported.size(); ++i) {
          out() << node->imported.at(i);
          if (i+1 < node->imported.size()) {
            out() << ", ";
          }
        }
      }
      out() << std::endl;
    }

    // #############
    // EXPR
    // #############

    void visit(BinaryOpExprNode *node) {
      out() << expr(node->n1) << " " << BinaryOpCodeToString(node->op) << " " << expr(node->n2);
    }
    void visit(UnaryOpExprNode *node) {
      out() << UnaryOpCodeToString(node->op) << expr(node->n1);
    }
    void visit(VarExprNode *node) {
      out() << "(var " << node->value << ")";
    }


    void visit(SimpleTypeExprNode *node) {
      out() << node->value;
    }
    void visit(ListTypeExprNode *node) {
      out() << "[]" << type(node->element);
    }
    void visit(MapTypeExprNode *node) {
      out() << "[" << type(node->key) << "]" << type(node->value);
    }
    void visit(TupleTypeExprNode *node) {
      out() << "(";
      for (int i = 0; i < node->elements.size(); ++i) {
        out() << type(node->elements.at(i));
        if (i + 1 == node->elements.size())
          out() << ", ";
      }
      out() << ")";
    }



    // #############
    // STATEMENT
    // #############
    void visit(ObjectDefNode *node) {
      indent() << "object " << type(node->type) << " " << cdata(node->id) << std::endl;
      scopedDecl(node->values);
      indent() << "end" << std::endl << std::endl;
    }
    void visit(PropertyDefNode *node) {
      indent() << "prop " << node->var << " " << cdata(node->value) << std::endl;
    }
    void visit(AtNode* node) {
      indent() << "at " << node->sender << " " << node->receiver << std::endl;
    }
    void visit(InterfaceDeclNode* node) {
      indent() << "interface " << node->name;
      if (node->inherits.size() > 0) {
        out() << "(";
        for (int i = 0; i < node->inherits.size(); ++i) {
          out() << node->inherits.at(i);
          if (i + 1 != node->inherits.size())
            out() << ", ";
        }
        out() << ")";
      }
      out() << std::endl;
      scopedDecl(node->values);
      indent() << "end" << std::endl << std::endl;
    }

    // a, ..., z
    void declParamList(const std::string& declname, const std::string& name, const TypeExprNodePtrVector& vec, const TypeExprNodePtr& ret = TypeExprNodePtr()) {
      indent() << declname << " " << name << "(";
      for (unsigned int i = 0; i < vec.size(); ++i) {
        out() << type(vec[i]);
        if (i+1 < vec.size()) {
          out() << ", ";
        }
      }
      out() << ")";
      if (ret)
        out() << " " << type(ret);
      out() << std::endl;
    }
    void visit(FnDeclNode* node) {
      declParamList("fn", node->name, node->args, node->ret);
    }
    void visit(EmitDeclNode* node) {
      declParamList("out", node->name, node->args);
    }
    void visit(PropDeclNode* node) {
      declParamList("prop", node->name, node->args);
    }
    void visit(StructDeclNode* node) {
      indent() << "struct " << node->name << std::endl;
      scopedDecl(node->values);
      indent() << "end" << std::endl << std::endl;
    }
    void visit(VarDefNode* node) {
      indent() << node->name;
      if (node->type)
        out() << " " << type(node->type);
      if (node->value)
        out() << " = " << cdata(node->value);
      out() << std::endl;
    }
    void visit(ConstDefNode* node) {
      indent() << "const " << node->name;
      if (node->type)
        out() << type(node->type);
      if (node->value)
        out() << " = " << cdata(node->value);
      out() << std::endl;
    }

  };

  std::string format(const NodePtr& node) {
    return QiLangFormatter().format(node);
  }

  std::string format(const NodePtrVector& node) {
    return QiLangFormatter().format(node);
  }

}
