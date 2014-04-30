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
  class ASTConstDataFormatter : public ConstDataNodeFormatter {
  public:
    virtual void acceptData(const ConstDataNodePtr& node) { node->accept(this); }

    const std::string &list(ConstDataNodePtrVector pv) {
      static const std::string ret;
      for (int i = 0; i < pv.size(); ++i) {
        acceptData(pv.at(i));
        if (i + 1 < pv.size())
          out() << " ";
      }
      return ret;
    }

    const std::string &dict(ConstDataNodePtrPairVector pv) {
      static const std::string ret;
      for (int i = 0; i < pv.size(); ++i) {
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
    void visitData(BoolConstDataNode *node) {
      out() << "(bool " << node->value << ")";
    }
    void visitData(IntConstDataNode *node) {
      out() << "(int " << node->value << ")";
    }
    void visitData(FloatConstDataNode *node) {
      out() << "(float " << node->value << ")";
    }
    void visitData(StringConstDataNode *node) {
      out() << "(string " << node->value << ")";
    }
    void visitData(ListConstDataNode* node) {
      out() << "(list " << list(node->values) << ")";
    }
    void visitData(TupleConstDataNode* node) {
      out() << "(tuple " << list(node->values) << ")";
    }
    void visitData(DictConstDataNode* node) {
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
      for (int i = 0; i < node->elements.size(); ++i) {
        acceptTypeExpr(node->elements.at(i));
        if (i + 1 == node->elements.size())
          out() << " ";
      }
      out() << ")";
    }
  };

  // #############
  // EXPR
  // #############
  class ASTExprFormatter : virtual public ASTConstDataFormatter, public ExprNodeFormatter {
  public:
    virtual void acceptExpr(const ExprNodePtr& node) { node->accept((ExprNodeVisitor*)this); }

    void visitExpr(BinaryOpExprNode *node) {
      out() << "(" << BinaryOpCodeToString(node->op) << " ";
      acceptExpr(node->n1);
      out() << " ";
      acceptExpr(node->n2);
      out() << ")";
    }
    void visitExpr(UnaryOpExprNode *node) {
      out() << "(" << UnaryOpCodeToString(node->op) << " ";
      acceptExpr(node->n1);
      out() << ")";
    }
    void visitExpr(VarExprNode *node) {
       out() << "(var " << node->value << ")";
    }
    void visitExpr(ConstDataExprNode* node) {
      acceptData(node->data);
    }
  };

  // #############
  // DECL
  // #############
  class ASTDeclFormatter: virtual public ASTTypeExprFormatter, virtual public ASTConstDataFormatter, public DeclNodeFormatter {
  public:
    virtual void acceptDecl(const DeclNodePtr& node) { node->accept((DeclNodeVisitor*)this); }

    void visitDecl(InterfaceDeclNode* node) {
      indent() << "(interface " << node->name;
      if (node->inherits.size() > 0) {
        out() << "(inherit ";
        for (int i = 0; i < node->inherits.size(); ++i) {
          out() << node->inherits.at(i);
          if (i + 1 != node->inherits.size())
            out() << " ";
        }
        out() << ")";
      }
      out() << std::endl;
      scopedDecl(node->values);
      indent() << ")" << std::endl;
    }

    void declParamList(const std::string &declname, const std::string& name, const TypeExprNodePtrVector& vec, const TypeExprNodePtr &ret = TypeExprNodePtr()) {
      out() << "(" << declname << " " << name << "(";
      for (unsigned int i = 0; i < vec.size(); ++i) {
        acceptTypeExpr(vec[i]);
        if (i+1 < vec.size()) {
          out() << " ";
        }
      }
      out() << ")";
      if (ret) {
        out() << " ";
        acceptTypeExpr(ret);
      }
      out() << ")" << std::endl;
    }

    void visitDecl(FnDeclNode* node) {
      declParamList("fn", node->name, node->args, node->ret);
    }
    void visitDecl(EmitDeclNode* node) {
      declParamList("out", node->name, node->args);
    }
    void visitDecl(PropDeclNode* node) {
      declParamList("prop", node->name, node->args);
    }

    void visitDecl(StructDeclNode* node) {
      indent() << "(struct " << node->name << std::endl;
      scopedField(node->fields);
      indent() << ")" << std::endl;
    }

    void visitDecl(FieldDeclNode* node) {
      indent() << "(field " << node->name;
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

  };

  class ASTStmtFormatter: virtual public ASTTypeExprFormatter, virtual public ASTConstDataFormatter, public StmtNodeFormatter {
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
        for (int i = 0; i < node->imports.size(); ++i) {
          out() << node->imports.at(i);
          if (i+1 < node->imports.size()) {
            out() << " ";
          }
        }
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
      indent() << "(at " << node->sender << " " << node->receiver << ")" << std::endl;
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
                           , public ASTExprFormatter
  {
  protected:
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
  };

  std::string formatAST(const NodePtr& node) {
    return QiLangASTFormatter().format(node);
  }

  std::string formatAST(const NodePtrVector& node) {
    return QiLangASTFormatter().format(node);
  }

}
