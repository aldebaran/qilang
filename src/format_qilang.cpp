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
#include "cpptype.hpp"

namespace qilang {

  // #############
  // CONST DATA
  // #############
  class QiLangLiteralFormatter : public LiteralNodeFormatter {
  public:
    virtual void acceptData(const LiteralNodePtr& node) { node->accept(this); }

    void list(LiteralNodePtrVector pv) {
      for (unsigned int i = 0; i < pv.size(); ++i) {
        acceptData(pv.at(i));
        if (i + 1 < pv.size())
          out() << ", ";
      }
    }

    void dict(LiteralNodePtrPairVector pv) {
      for (unsigned int i = 0; i < pv.size(); ++i) {
        acceptData(pv.at(i).first);
        out() << " : ";
        acceptData(pv.at(i).second);
        if (i + 1 < pv.size())
          out() << ", ";
      }
    }

    void visitData(BoolLiteralNode *node) {
      if (node->value)
        out() << "true";
      else
        out() << "false";
    }
    void visitData(IntLiteralNode *node) {
      out() << node->value;
    }
    void visitData(FloatLiteralNode *node) {
      out() << node->value;
    }
    void visitData(StringLiteralNode *node) {
      out() << node->value;
    }
    void visitData(ListLiteralNode* node) {
      out() << "[ ";
      list(node->values);
      out() << " ]";
    }
    void visitData(TupleLiteralNode* node) {
      out() << "( ";
      list(node->values);
      out() << " )";
    }
    void visitData(DictLiteralNode* node) {
      out() << "{ ";
      dict(node->values);
      out() << " }";
    }
  };

  // #############
  // TYPE EXPR
  // #############
  class QiLangTypeExprFormatter : public TypeExprNodeFormatter {
  public:
    virtual void acceptTypeExpr(const TypeExprNodePtr& node)  { node->accept((TypeExprNodeVisitor*)this); }

    void visitTypeExpr(BuiltinTypeExprNode *node) {
      out() << node->value;
    }
    void visitTypeExpr(CustomTypeExprNode *node) {
      out() << node->value;
    }
    void visitTypeExpr(ListTypeExprNode *node) {
      out() << "[]";
      acceptTypeExpr(node->element);
    }
    void visitTypeExpr(MapTypeExprNode *node) {
      out() << "[";
      acceptTypeExpr(node->key);
      out() << "]";
      acceptTypeExpr(node->value);
    }
    void visitTypeExpr(TupleTypeExprNode *node) {
      out() << "(";
      for (unsigned int i = 0; i < node->elements.size(); ++i) {
        acceptTypeExpr(node->elements.at(i));
        if (i + 1 == node->elements.size())
          out() << ", ";
      }
      out() << ")";
    }
  };

  // #############
  // EXPR
  // #############
  class QiLangExprFormatter : virtual public QiLangLiteralFormatter, public ExprNodeFormatter {
  public:
    virtual void acceptExpr(const ExprNodePtr& node) { node->accept((ExprNodeVisitor*)this); }

    void visitExpr(BinaryOpExprNode *node) {
      acceptExpr(node->left);
      out() << " " << BinaryOpCodeToString(node->op) << " ";
      acceptExpr(node->right);
    }
    void visitExpr(UnaryOpExprNode *node) {
      out() << UnaryOpCodeToString(node->op);
      acceptExpr(node->expr);
    }
    void visitExpr(VarExprNode *node) {
      out() << node->value;
    }
    void visitExpr(LiteralExprNode* node) {
      acceptData(node->data);
    }
  };

  // #############
  // DECL
  // #############
  class QiLangDeclFormatter: virtual public QiLangTypeExprFormatter, virtual public QiLangLiteralFormatter, public DeclNodeFormatter {
  public:
    virtual void acceptDecl(const DeclNodePtr& node) { node->accept((DeclNodeVisitor*)this); }

    // a, ..., z
    void declParamList(const std::string& declname, const std::string& name, const TypeExprNodePtrVector& vec, const TypeExprNodePtr& ret = TypeExprNodePtr()) {
      indent() << declname << " " << name << "(";
      for (unsigned int i = 0; i < vec.size(); ++i) {
        acceptTypeExpr(vec[i]);
        if (i+1 < vec.size()) {
          out() << ", ";
        }
      }
      out() << ")";
      if (ret) {
        out() << " ";
        acceptTypeExpr(ret);
      }
      out() << std::endl;
    }

    void printInherits(const StringVector& inherits) {
      if (inherits.size() > 0) {
        out() << "(";
        for (unsigned int i = 0; i < inherits.size(); ++i) {
          out() << inherits.at(i);
          if (i + 1 != inherits.size())
            out() << ", ";
        }
        out() << ")";
      }
    }

    void visitDecl(InterfaceDeclNode* node) {
      indent() << "interface " << node->name;
      printInherits(node->inherits);
      out() << std::endl;
      scopedDecl(node->values);
      indent() << "end" << std::endl << std::endl;
    }

    void visitDecl(FnDeclNode* node) {
      declParamList("fn", node->name, node->args, node->ret);
    }
    void visitDecl(EmitDeclNode* node) {
      declParamList("emit", node->name, node->args);
    }
    void visitDecl(PropDeclNode* node) {
      declParamList("prop", node->name, node->args);
    }
    void visitDecl(StructDeclNode* node) {
      indent() << "struct " << node->name;
      printInherits(node->inherits);
      out() << std::endl;
      scopedDecl(node->decls);
      indent() << "end" << std::endl << std::endl;
    }
    void visitDecl(ConstDeclNode* node) {
      indent() << "const " << node->name;
      if (node->type)
        acceptTypeExpr(node->type);
      if (node->data) {
        out() << " = ";
        acceptData(node->data);
      }
      out() << std::endl;
    }
    void visitDecl(StructFieldDeclNode* node) {
      indent() << node->name;
      if (node->type) {
        out() << " ";
        acceptTypeExpr(node->type);
      }
      out() << std::endl;
    }

    void visitDecl(EnumDeclNode* node) {
      indent() << "enum " << node->name << std::endl;
      scopedEnumField(node->fields);
      indent() << "end" << std::endl;
    }
    void visitDecl(TypeDefDeclNode* node) {
      indent() << "typedef ";
      acceptTypeExpr(node->type);
      out() << " " << node->name << std::endl;
    }
    void visitDecl(EnumFieldDeclNode* node) {
      indent() << "TODO" << std::endl;
    }
  };

  class QiLangStmtFormatter: virtual public QiLangTypeExprFormatter, virtual public QiLangLiteralFormatter, public StmtNodeFormatter {
  public:
    virtual void acceptStmt(const StmtNodePtr& node) { node->accept((StmtNodeVisitor*)this); }

    // #############
    // STATEMENT
    // #############
    void visitStmt(PackageNode* node) {
      indent() << "package " << node->name << std::endl;
    }
    void visitStmt(ImportNode* node) {
      if (node->importType == ImportType_Package)
        indent() << "import " << node->name;
      else if (node->importType == ImportType_All)
        indent() << "from " << node->name << " import *";
      else {
        indent() << "from " << node->name << " import ";
        for (unsigned int i = 0; i < node->imports.size(); ++i) {
          out() << node->imports.at(i);
          if (i+1 < node->imports.size()) {
            out() << ", ";
          }
        }
      }
      out() << std::endl;
    }
    void visitStmt(ObjectDefNode *node) {
      indent() << "object ";
      acceptTypeExpr(node->type);
      out() << " " << node->name << std::endl;
      scopedStmt(node->values);
      indent() << "end" << std::endl << std::endl;
    }
    void visitStmt(PropertyDefNode *node) {
      indent() << "prop " << node->name << " ";
      acceptData(node->data);
      out() << std::endl;
    }
    void visitStmt(AtNode* node) {
      indent() << "at " << node->sender << ": " << node->receiver << std::endl;
    }
    void visitStmt(VarDefNode* node) {
      indent() << node->name;
      if (node->type) {
        out() << " ";
        acceptTypeExpr(node->type);
      }
      if (node->data) {
        out() << " = ";
        acceptData(node->data);
      }
      out() << std::endl;
    }
    void visitStmt(CommentNode* node) {
      formatBlock(out(), node->comments, "# ", _indent);
    }

  };

  class QiLangFormatter : public FileFormatter
                        , public QiLangStmtFormatter
                        , public QiLangExprFormatter
                        , public QiLangDeclFormatter
  {
  protected:
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

  };

  std::string format(const NodePtr& node) {
    return QiLangFormatter().format(node);
  }

  std::string format(const NodePtrVector& node) {
    return QiLangFormatter().format(node);
  }

}
