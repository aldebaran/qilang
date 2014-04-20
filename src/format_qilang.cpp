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

  // #############
  // CONST DATA
  // #############
  class QiLangConstDataFormatter : public ConstDataNodeFormatter {
  public:
    virtual void acceptData(const ConstDataNodePtr& node) { node->accept(this); }

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

    virtual void visitData(BoolConstDataNode *node) {
      if (node->value)
        out() << "true";
      else
        out() << "false";
    }
    virtual void visitData(IntConstDataNode *node) {
      out() << node->value;
    }
    virtual void visitData(FloatConstDataNode *node) {
      out() << node->value;
    }
    virtual void visitData(StringConstDataNode *node) {
      out() << node->value;
    }
    virtual void visitData(ListConstDataNode* node) {
      out() << "[ " << list(node->values) << " ]";
    }
    virtual void visitData(TupleConstDataNode* node) {
      out() << "( " << list(node->values) << " )";
    }
    virtual void visitData(DictConstDataNode* node) {
      out() << "{ " << dict(node->values) << " }";
    }
  };

  // #############
  // TYPE EXPR
  // #############
  class QiLangTypeExprFormatter : public TypeExprNodeFormatter {
  public:
    virtual void acceptTypeExpr(const TypeExprNodePtr& node)  { node->accept((TypeExprNodeVisitor*)this); }
    void visitTypeExpr(SimpleTypeExprNode *node) {
      out() << node->value;
    }
    void visitTypeExpr(ListTypeExprNode *node) {
      out() << "[]" << type(node->element);
    }
    void visitTypeExpr(MapTypeExprNode *node) {
      out() << "[" << type(node->key) << "]" << type(node->value);
    }
    void visitTypeExpr(TupleTypeExprNode *node) {
      out() << "(";
      for (int i = 0; i < node->elements.size(); ++i) {
        out() << type(node->elements.at(i));
        if (i + 1 == node->elements.size())
          out() << ", ";
      }
      out() << ")";
    }
  };

  // #############
  // EXPR
  // #############
  class QiLangExprFormatter : public QiLangConstDataFormatter, public ExprNodeFormatter {
  public:
    virtual void acceptExpr(const ExprNodePtr& node) { node->accept((ExprNodeVisitor*)this); }

    void visitExpr(BinaryOpExprNode *node) {
      out() << expr(node->n1) << " " << BinaryOpCodeToString(node->op) << " " << expr(node->n2);
    }
    void visitExpr(UnaryOpExprNode *node) {
      out() << UnaryOpCodeToString(node->op) << expr(node->n1);
    }
    void visitExpr(VarExprNode *node) {
      out() << node->value;
    }
    void visitExpr(ConstDataExprNode* node) {
      out() << cdata(node->data);
    }
  };

  class QiLangFormatter : public FileFormatter
                        , public StmtNodeFormatter
                        , public QiLangTypeExprFormatter
                        , public QiLangExprFormatter
  {
  protected:
    virtual void accept(const NodePtr& node) {
      switch (node->kind()) {
      case NodeKind_ConstData:
        acceptData(boost::dynamic_pointer_cast<ConstDataNode>(node));
        break;
      case NodeKind_Decl:
        acceptStmt(boost::dynamic_pointer_cast<StmtNode>(node));
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

    virtual void acceptStmt(const StmtNodePtr& node)      { node->accept((StmtNodeVisitor*)this); }

    virtual void visitStmt(PackageNode* node) {
      indent() << "package " << node->name << std::endl;
    }

    void visitStmt(ImportNode* node) {
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
    // STATEMENT
    // #############
    void visitStmt(ObjectDefNode *node) {
      indent() << "object " << type(node->type) << " " << cdata(node->name) << std::endl;
      //TODO scopedDecl(node->values);
      indent() << "end" << std::endl << std::endl;
    }
    void visitStmt(PropertyDefNode *node) {
      indent() << "prop " << node->name << " " << cdata(node->data) << std::endl;
    }
    void visitStmt(AtNode* node) {
      indent() << "at " << node->sender << " " << node->receiver << std::endl;
    }
    void visitStmt(InterfaceDeclNode* node) {
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
    void visitStmt(FnDeclNode* node) {
      declParamList("fn", node->name, node->args, node->ret);
    }
    void visitStmt(EmitDeclNode* node) {
      declParamList("out", node->name, node->args);
    }
    void visitStmt(PropDeclNode* node) {
      declParamList("prop", node->name, node->args);
    }
    void visitStmt(StructDeclNode* node) {
      indent() << "struct " << node->name << std::endl;
      //TODO scopedDecl(node->values);
      indent() << "end" << std::endl << std::endl;
    }
    void visitStmt(VarDefNode* node) {
      indent() << node->name;
      if (node->type)
        out() << " " << type(node->type);
      if (node->data)
        out() << " = " << cdata(node->data);
      out() << std::endl;
    }
    void visitStmt(ConstDefNode* node) {
      indent() << "const " << node->name;
      if (node->type)
        out() << type(node->type);
      if (node->data)
        out() << " = " << cdata(node->data);
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
