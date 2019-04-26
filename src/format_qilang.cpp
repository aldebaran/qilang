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
  class QiLangFormatter : public NodeFormatter<>
  {
    virtual void doAccept(Node* node) { node->accept(this); }

    void dict(LiteralNodePtrPairVector pv) {
      for (unsigned int i = 0; i < pv.size(); ++i) {
        accept(pv.at(i).first);
        out() << " : ";
        accept(pv.at(i).second);
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
      out() << "\"" << node->value << "\"";
    }
    void visitData(ListLiteralNode* node) {
      out() << "[ ";
      join(node->values, ", ");
      out() << " ]";
    }
    void visitData(TupleLiteralNode* node) {
      out() << "( ";
      join(node->values, ", ");
      out() << " )";
    }
    void visitData(DictLiteralNode* node) {
      out() << "{ ";
      dict(node->values);
      out() << " }";
    }

    void visitTypeExpr(BuiltinTypeExprNode *node) {
      out() << node->value;
    }
    void visitTypeExpr(CustomTypeExprNode *node) {
      out() << node->value;
    }
    void visitTypeExpr(ListTypeExprNode *node) {
      out() << "[]";
      accept(node->element);
    }
    void visitTypeExpr(MapTypeExprNode *node) {
      out() << "[";
      accept(node->key);
      out() << "]";
      accept(node->value);
    }
    void visitTypeExpr(TupleTypeExprNode *node) {
      out() << "(";
      join(node->elements, ", ");
      out() << ")";
    }
    void visitTypeExpr(OptionalTypeExprNode *node) {
      out() << "+"; // like in libqi
      accept(node->element);
    }
    void visitTypeExpr(VarArgTypeExprNode* node) {
      accept(node->effectiveElement());
    }
    void visitTypeExpr(KeywordArgTypeExprNode* node) {
      accept(node->effectiveValue());
    }

    void visitExpr(BinaryOpExprNode *node) {
      accept(node->left);
      out() << " " << BinaryOpCodeToString(node->op) << " ";
      accept(node->right);
    }
    void visitExpr(UnaryOpExprNode *node) {
      out() << UnaryOpCodeToString(node->op);
      accept(node->expr);
    }
    void visitExpr(VarExprNode *node) {
      out() << node->value;
    }
    void visitExpr(LiteralExprNode* node) {
      accept(node->data);
    }
    void visitExpr(CallExprNode* node) {
      out() << node->name << "(";
      join(node->args, ", ");
      out() << ")";
    }

    // a, ..., z
    void declParamList(const std::string& declname, const std::string& name, const std::string& comment, const ParamFieldDeclNodePtrVector& vec, const TypeExprNodePtr& ret = TypeExprNodePtr()) {
      if (!comment.empty())
        indent() << comment << std::endl;
      indent() << declname << " " << name << "(";
      join(vec, ", ");
      out() << ")";
      if (ret) {
        out() << " -> ";
        accept(ret);
      }
      out() << std::endl << std::endl;
      }

    void declParamList(const std::string& declname, const std::string& name, const ParamFieldDeclNodePtrVector& vec, const TypeExprNodePtr& ret = TypeExprNodePtr()) {
      indent() << declname << " " << name << "(";
      join(vec, ", ");
      out() << ")";
      if (ret) {
        out() << " -> ";
        accept(ret);
      }
      out() << std::endl << std::endl;
    }

    void printInherits(const StringVector& inherits) {
      if (inherits.size() > 0) {
        out() << "(";
        join(inherits, ", ");
        out() << ")";
      }
    }

    void visitDecl(InterfaceDeclNode* node) {
      indent() << "interface " << node->name;
      printInherits(node->inherits);
      out() << std::endl;
      scoped(node->values);
      indent() << "end" << std::endl << std::endl;
    }

    void visitDecl(FnDeclNode* node) {
      declParamList("fn", node->name, node->comment(), node->args, node->ret);
    }
    void visitDecl(SigDeclNode* node) {
      declParamList("emit", node->name, node->args);
    }
    void visitDecl(PropDeclNode* node) {
      declParamList("prop", node->name, node->args);
    }
    void visitDecl(ParamFieldDeclNode* node) {
      if (node->isVarArgs())
        out() << "*";
      if (node->isKeywordArgs())
        out() << "**";
      if (node->names.size() > 1) {
        out() << "(";
        join(node->names, ", ");
        out() << ")";
      } else {
        out() << node->names.at(0);
      }
      if (node->type) {
        out() << " ";
        accept(node->type);
      }
    }

    void visitDecl(StructDeclNode* node) {
      indent() << "struct " << node->name;
      printInherits(node->inherits);
      out() << std::endl;
      scoped(node->decls);
      indent() << "end" << std::endl << std::endl;
    }
    void visitDecl(ConstDeclNode* node) {
      indent() << "const " << node->name;
      if (node->type)
        accept(node->type);
      if (node->data) {
        out() << " = ";
        accept(node->data);
      }
      out() << std::endl;
    }
    void visitDecl(StructFieldDeclNode* node) {
      indent() << "";
      join(node->names, ", ");
      if (node->type) {
        out() << " ";
        accept(node->type);
      }
      out() << std::endl;
    }

    void visitDecl(EnumDeclNode* node) {
      indent() << "enum " << node->name << std::endl;
      scoped(node->fields);
      indent() << "end" << std::endl;
    }
    void visitDecl(TypeDefDeclNode* node) {
      indent() << "typedef ";
      accept(node->type);
      out() << " " << node->name << std::endl;
    }
    void visitDecl(EnumFieldDeclNode* node) {
      indent() << "TODO" << std::endl;
    }

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
        join(node->imports, ", ");
      }
      out() << std::endl;
    }

    void visitStmt(PropertyDefNode *node) {
      indent() << "prop " << node->name << " ";
      accept(node->data);
      out() << std::endl;
    }

    void visitStmt(VarDefNode* node) {
      indent() << node->name;
      if (node->type) {
        out() << " ";
        accept(node->type);
      }
      if (node->data) {
        out() << " = ";
        accept(node->data);
      }
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
