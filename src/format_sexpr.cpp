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
  class QiLangASTFormatter : public NodeFormatter<>
  {
    virtual void doAccept(Node* node) { node->accept(this); }

    const std::string &dict(LiteralNodePtrPairVector pv) {
      static const std::string ret;
      for (unsigned int i = 0; i < pv.size(); ++i) {
        out() << "(";
        accept(pv.at(i).first);
        out() << " ";
        accept(pv.at(i).second);
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
      join(node->values, " ");
      out() << ")";
    }
    void visitData(TupleLiteralNode* node) {
      out() << "(tuple ";
      join(node->values, " ");
      out() << ")";
    }
    void visitData(DictLiteralNode* node) {
      out() << "(dict " << dict(node->values) << ")";
    }

    void visitTypeExpr(BuiltinTypeExprNode *node) {
      out() << "(btype " << node->value << ")";
    }
    void visitTypeExpr(CustomTypeExprNode *node) {
      out() << "(type " << node->value << ")";
    }
    void visitTypeExpr(ListTypeExprNode *node) {
      out() << "(listtype ";
      accept(node->element);
      out() << ")";
    }
    void visitTypeExpr(MapTypeExprNode *node) {
      out() << "(maptype ";
      accept(node->key);
      out() << " ";
      accept(node->value);
      out() << ")";
    }
    void visitTypeExpr(TupleTypeExprNode *node) {
      out() << "(tupletype ";
      join(node->elements, " ");
      out() << ")";
    }
    void visitTypeExpr(VarArgTypeExprNode* node) {
      out() << "(varg ";
      accept(node->element);
      out() << ")";
    }
    void visitTypeExpr(KeywordArgTypeExprNode* node) {
      out() << "(kwarg ";
      accept(node->value);
      out() << ")";
    }

    void visitExpr(BinaryOpExprNode *node) {
      out() << "(" << BinaryOpCodeToString(node->op) << " ";
      accept(node->left);
      out() << " ";
      accept(node->right);
      out() << ")";
    }
    void visitExpr(UnaryOpExprNode *node) {
      out() << "(" << UnaryOpCodeToString(node->op) << " ";
      accept(node->expr);
      out() << ")";
    }
    void visitExpr(VarExprNode *node) {
       out() << "(var " << node->value << ")";
    }
    void visitExpr(LiteralExprNode* node) {
      accept(node->data);
    }
    void visitExpr(CallExprNode* node) {
      out() << node->name << "(";
      join(node->args, " ");
      out() << ")";
    }

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
      scoped(node->values);
      indent() << ")" << std::endl;
    }

    void declParamList(const std::string &declname, const std::string& name, const ParamFieldDeclNodePtrVector& vec, const TypeExprNodePtr &ret = TypeExprNodePtr()) {
      out() << "(" << declname << " " << name << "(";
      join(vec, " ");
      out() << ")";
      if (ret) {
        out() << " ";
        accept(ret);
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
        accept(node->type);
        out() << ")";
      }
      out() << ")";
    }
    void visitDecl(FnDeclNode* node) {
      declParamList("fn", node->name, node->args, node->ret);
    }
    void visitDecl(SigDeclNode* node) {
      declParamList("emit", node->name, node->args);
    }
    void visitDecl(PropDeclNode* node) {
      declParamList("prop", node->name, node->args);
    }
    void visitDecl(StructDeclNode* node) {
      indent() << "(struct " << node->name;
      printInherit(node->inherits);
      out() << std::endl;
      scoped(node->decls);
      indent() << ")" << std::endl;
    }
    void visitDecl(StructFieldDeclNode* node) {
      indent() << "(field (";
      join(node->names, " ");
      if (node->type) {
        out() << " ";
        accept(node->type);
      }
      out() << ")" << std::endl;
    }
    void visitDecl(ConstDeclNode* node) {
      indent() << "(defconst " << node->name;
      if (node->type) {
        out() << " ";
        accept(node->type);
      }
      if (node->data) {
        out() << " ";
        accept(node->data);
      }
      out() << ")" << std::endl;
    }
    void visitDecl(EnumDeclNode* node) {
      indent() << "(enum " << node->name << std::endl;
      scoped(node->fields);
      indent() << ")" << std::endl;
    }
    void visitDecl(TypeDefDeclNode* node) {
      indent() << "(typedef ";
      accept(node->type);
      out() << " " << node->name << ")" << std::endl;
    }
    void visitDecl(EnumFieldDeclNode* node) {
      indent() << "(enum " << std::endl;
      if (node->fieldType == EnumFieldType_Const) {
        visitDecl(static_cast<ConstDeclNode*>(node->node.get()));
      } else {
        accept(boost::static_pointer_cast<TypeExprNode>(node->node));
      }
      indent() << ")" << std::endl;
    }

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

    void visitStmt(PropertyDefNode *node) {
      indent() << "(prop " << node->name << " ";
      accept(node->data);
      out() << ")" << std::endl;
    }

    void visitStmt(VarDefNode* node) {
      indent() << "(defvar " << node->name;
      if (node->type) {
        out() << " ";
        accept(node->type);
      }
      if (node->data) {
        out() << " ";
        accept(node->data);
      }
      out() << ")" << std::endl;
    }

  };

  std::string formatAST(const NodePtr& node) {
    return QiLangASTFormatter().format(node);
  }

  std::string formatAST(const NodePtrVector& node) {
    return QiLangASTFormatter().format(node);
  }

}
