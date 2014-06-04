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
  class ASTLiteralFormatter : public LiteralNodeFormatter {
  public:
    virtual void acceptData(const LiteralNodePtr& node) { node->accept(this); }

    const std::string &dict(LiteralNodePtrPairVector pv) {
      static const std::string ret;
      for (unsigned int i = 0; i < pv.size(); ++i) {
        out() << "(";
        acceptData(pv.at(i).first);
        out() << " ";
        acceptData(pv.at(i).second);
        out() << ")";
        if (i + 1 < pv.size())
          out() << " ";
      }
      return ret;
    }
    void visitData(BoolLiteralNode *node) {
      out() << "(bool " << node->value << ")";
    }
    void visitData(IntLiteralNode *node) {
      out() << "(int " << node->value << ")";
    }
    void visitData(FloatLiteralNode *node) {
      out() << "(float " << node->value << ")";
    }
    void visitData(StringLiteralNode *node) {
      out() << "(string " << node->value << ")";
    }
    void visitData(ListLiteralNode* node) {
      out() << "(list ";
      joinLiteral(node->values, " ");
      out() << ")";
    }
    void visitData(TupleLiteralNode* node) {
      out() << "(tuple ";
      joinLiteral(node->values, " ");
      out() << ")";
    }
    void visitData(DictLiteralNode* node) {
      out() << "(dict " << dict(node->values) << ")";
    }
  };

  // #############
  // TYPE EXPR
  // #############
  class ASTTypeExprFormatter : public TypeExprNodeFormatter {
  public:
    virtual void acceptTypeExpr(const TypeExprNodePtr& node)  { node->accept((TypeExprNodeVisitor*)this); }
    void visitTypeExpr(BuiltinTypeExprNode *node) {
      out() << "(btype " << node->value << ")";
    }
    void visitTypeExpr(CustomTypeExprNode *node) {
      out() << "(type " << node->value << ")";
    }
    void visitTypeExpr(ListTypeExprNode *node) {
      out() << "(listtype ";
      acceptTypeExpr(node->element);
      out() << ")";
    }
    void visitTypeExpr(MapTypeExprNode *node) {
      out() << "(maptype ";
      acceptTypeExpr(node->key);
      out() << " ";
      acceptTypeExpr(node->value);
      out() << ")";
    }
    void visitTypeExpr(TupleTypeExprNode *node) {
      out() << "(tupletype ";
      joinTypeExpr(node->elements, " ");
      out() << ")";
    }
  };

  // #############
  // EXPR
  // #############
  class ASTExprFormatter : virtual public ASTLiteralFormatter, public ExprNodeFormatter {
  public:
    virtual void acceptExpr(const ExprNodePtr& node) { node->accept((ExprNodeVisitor*)this); }

    void visitExpr(BinaryOpExprNode *node) {
      out() << "(" << BinaryOpCodeToString(node->op) << " ";
      acceptExpr(node->left);
      out() << " ";
      acceptExpr(node->right);
      out() << ")";
    }
    void visitExpr(UnaryOpExprNode *node) {
      out() << "(" << UnaryOpCodeToString(node->op) << " ";
      acceptExpr(node->expr);
      out() << ")";
    }
    void visitExpr(VarExprNode *node) {
       out() << "(var " << node->value << ")";
    }
    void visitExpr(LiteralExprNode* node) {
      acceptData(node->data);
    }
    void visitExpr(CallExprNode* node) {
      out() << node->name << "(";
      joinExpr(node->args, " ");
      out() << ")";
    }

  };

  // #############
  // DECL
  // #############
  class ASTDeclFormatter: virtual public ASTTypeExprFormatter, virtual public ASTLiteralFormatter, public DeclNodeFormatter {
  public:
    virtual void acceptDecl(const DeclNodePtr& node) { node->accept((DeclNodeVisitor*)this); }

    void printInherit(const StringVector& inherits) {
      if (inherits.size() > 0) {
        out() << "(inherit ";
        join(inherits, " ");
        out() << ")";
      }
    }

    void visitDecl(InterfaceDeclNode* node) {
      indent() << "(interface " << node->name;
      printInherit(node->inherits);
      out() << std::endl;
      scopedDecl(node->values);
      indent() << ")" << std::endl;
    }

    void declParamList(const std::string &declname, const std::string& name, const ParamFieldDeclNodePtrVector& vec, const TypeExprNodePtr &ret = TypeExprNodePtr()) {
      out() << "(" << declname << " " << name << "(";
      joinDecl(vec, " ");
      out() << ")";
      if (ret) {
        out() << " ";
        acceptTypeExpr(ret);
      }
      out() << ")" << std::endl;
    }

    void visitDecl(ParamFieldDeclNode* node) {
      if (node->isVarArgs())
        out() << "(varg ";
      else if (node->isKeywordArgs())
        out() << "(kwarg ";
      else
        out() << "(arg ";
      join(node->names, " ");
      if (node->type) {
        out() << " (";
        acceptTypeExpr(node->type);
        out() << ")";
      }
      out() << ")";
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
      indent() << "(struct " << node->name;
      printInherit(node->inherits);
      out() << std::endl;
      scopedDecl(node->decls);
      indent() << ")" << std::endl;
    }

    void visitDecl(StructFieldDeclNode* node) {
      indent() << "(field (";
      join(node->names, " ");
      if (node->type) {
        out() << " ";
        acceptTypeExpr(node->type);
      }
      out() << ")" << std::endl;
    }

    void visitDecl(ConstDeclNode* node) {
      indent() << "(defconst " << node->name;
      if (node->type) {
        out() << " ";
        acceptTypeExpr(node->type);
      }
      if (node->data) {
        out() << " ";
        acceptData(node->data);
      }
      out() << ")" << std::endl;
    }
    void visitDecl(EnumDeclNode* node) {
      indent() << "(enum " << node->name << std::endl;
      scopedEnumField(node->fields);
      indent() << ")" << std::endl;
    }
    void visitDecl(TypeDefDeclNode* node) {
      indent() << "(typedef ";
      acceptTypeExpr(node->type);
      out() << " " << node->name << ")" << std::endl;
    }
    void visitDecl(EnumFieldDeclNode* node) {
      indent() << "(enum " << std::endl;
      if (node->fieldType == EnumFieldType_Const) {
        visitDecl(static_cast<ConstDeclNode*>(node->node.get()));
      } else {
        acceptTypeExpr(boost::static_pointer_cast<TypeExprNode>(node->node));
      }
      indent() << ")" << std::endl;
    }


  };

  class ASTStmtFormatter: virtual public ASTExprFormatter, virtual public ASTTypeExprFormatter, virtual public ASTLiteralFormatter, public StmtNodeFormatter {
  public:
    virtual void acceptStmt(const StmtNodePtr& node) { node->accept((StmtNodeVisitor*)this); }

    void visitStmt(PackageNode* node) {
      indent() << "(package " << node->name << ")" << std::endl;
    }

    void visitStmt(ImportNode* node) {
      if (node->importType == ImportType_Package) {
        indent() << "(import " << node->name << ")" << std::endl;
      } else if (node->importType == ImportType_All) {
        indent() << "(from " << node->name << " (import all))" << std::endl;
      } else {
        indent() << "(from " << node->name << " (import ";
        join(node->imports, " ");
        out() << "))" << std::endl;
      }
    }

    void visitStmt(ObjectDefNode *node) {
      indent() << "(object ";
      acceptTypeExpr(node->type);
      out() << " " << node->name << std::endl;
      scopedStmt(node->values);
      indent() << ")" << std::endl;
    }
    void visitStmt(PropertyDefNode *node) {
      indent() << "(prop " << node->name << " ";
      acceptData(node->data);
      out() << ")" << std::endl;
    }
    void visitStmt(AtNode* node) {
      indent() << "(at ";
      acceptExpr(node->_sender);
      out() << " " << node->receiver << ")" << std::endl;
    }
    void visitStmt(VarDefNode* node) {
      indent() << "(defvar " << node->name;
      if (node->type) {
        out() << " ";
        acceptTypeExpr(node->type);
      }
      if (node->data) {
        out() << " ";
        acceptData(node->data);
      }
      out() << ")" << std::endl;
    }
    void visitStmt(CommentNode* node) {
      formatBlock(out(), node->comments, ";; ", _indent);
    }


  };

  class QiLangASTFormatter : public FileFormatter
                           , public ASTStmtFormatter
                           , public ASTDeclFormatter
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

  std::string formatAST(const NodePtr& node) {
    return QiLangASTFormatter().format(node);
  }

  std::string formatAST(const NodePtrVector& node) {
    return QiLangASTFormatter().format(node);
  }

}
