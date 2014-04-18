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

  class QiLangFormatter : public NodeFormatter, public NodeVisitor {
  protected:
    virtual void accept(const NodePtr& node) { node->accept(this); }

    //indented block
    void scopedDecl(const qilang::NodePtrVector& vec) {
      ScopedIndent _(_indent);
      for (unsigned int i = 0; i < vec.size(); ++i) {
        vec[i]->accept(this);
      }
    }
    void visit(PackageNode* node) {
      indent() << "package " << expr(node->name) << std::endl;
    }

    void visit(ImportNode* node) {
      if (node->imported.size() == 0)
        indent() << "import " << expr(node->name);
      else {
        indent() << "from " << expr(node->name) << " import ";
        for (unsigned int i = 0; i < node->imported.size(); ++i) {
          out() << expr(node->imported.at(i));
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

    const std::string &list(ConstExprNodePtrVector pv) {
      static const std::string ret;
      for (int i = 0; i < pv.size(); ++i) {
        expr(pv.at(i));
        if (i + 1 < pv.size())
          out() << ", ";
      }
      return ret;
    }

    const std::string &dict(ConstExprNodePtrPairVector pv) {
      static const std::string ret;
      for (int i = 0; i < pv.size(); ++i) {
        out() << expr(pv.at(i).first) << " : " << expr(pv.at(i).second);
        if (i + 1 < pv.size())
          out() << ", ";
      }
      return ret;
    }

    void visit(IntConstNode *node) {
      out() << node->value;
    }
    void visit(FloatConstNode *node) {
      out() << node->value;
    }
    void visit(StringConstNode *node) {
      out() << node->value;
    }
    void visit(ListConstNode* node) {
      out() << "[ " << list(node->values) << " ]";
    }
    void visit(TupleConstNode* node) {
      out() << "( " << list(node->values) << " )";
    }
    void visit(DictConstNode* node) {
      out() << "{ " << dict(node->values) << " }";
    }
    void visit(SymbolNode* node) {
      out() << node->name;
    }
    void visit(BinaryOpNode *node) {
      out() << expr(node->n1) << " " << BinaryOpCodeToString(node->op) << " " << expr(node->n2);
    }
    void visit(UnaryOpNode *node) {
      out() << UnaryOpCodeToString(node->op) << expr(node->n1);
    }
    void visit(ExprNode *node) {
      out() << "(" << expr(node->value) << ")" << std::endl;
    }
    void visit(SimpleTypeNode *node) {
      out() << node->value;
    }
    void visit(ListTypeNode *node) {
      out() << "[]" << expr(node->element);
    }
    void visit(MapTypeNode *node) {
      out() << "[" << expr(node->key) << "]" << expr(node->value);
    }
    void visit(TupleTypeNode *node) {
      out() << "(";
      for (int i = 0; i < node->elements.size(); ++i) {
        out() << expr(node->elements.at(i));
        if (i + 1 == node->elements.size())
          out() << ", ";
      }
      out() << ")";
    }
    void visit(VarNode *node) {
      out() << "(var " << expr(node->value) << ")";
    }


    // #############
    // STATEMENT
    // #############
    void visit(ObjectNode *node) {
      indent() << "object " << expr(node->type) << " " << expr(node->id) << std::endl;
      scopedDecl(node->values);
      indent() << "end" << std::endl << std::endl;
    }
    void visit(PropertyNode *node) {
      indent() << "prop " << expr(node->var) << " " << expr(node->value) << std::endl;
    }
    void visit(AtNode* node) {
      indent() << "at " << expr(node->sender) << " " << expr(node->receiver) << std::endl;
    }
    void visit(InterfaceDeclNode* node) {
      indent() << "interface " << expr(node->name);
      if (node->inherits.size() > 0) {
        out() << "(";
        for (int i = 0; i < node->inherits.size(); ++i) {
          out() << expr(node->inherits.at(i));
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
    void declParamList(const std::string& declname, const SymbolNodePtr& name, const TypeNodePtrVector& vec, const TypeNodePtr& ret = TypeNodePtr()) {
      indent() << declname << " " << expr(name) << "(";
      for (unsigned int i = 0; i < vec.size(); ++i) {
        out() << expr(vec[i]);
        if (i+1 < vec.size()) {
          out() << ", ";
        }
      }
      out() << ")";
      if (ret)
        out() << " " << expr(ret);
      out() << std::endl;
    }
    void visit(FnDeclNode* node) {
      declParamList("fn", node->name, node->args, node->ret);
    }
    void visit(InDeclNode* node) {
      declParamList("in", node->name, node->args);
    }
    void visit(OutDeclNode* node) {
      declParamList("out", node->name, node->args);
    }
    void visit(PropDeclNode* node) {
      declParamList("prop", node->name, node->args);
    }
    void visit(StructNode* node) {
      indent() << "struct " << expr(node->name) << std::endl;
      scopedDecl(node->values);
      indent() << "end" << std::endl << std::endl;
    }
    void visit(VarDefNode* node) {
      indent() << expr(node->name);
      if (node->type)
        out() << " " << expr(node->type);
      if (node->value)
        out() << " = " << expr(node->value);
      out() << std::endl;
    }
    void visit(ConstDefNode* node) {
      indent() << "const " << expr(node->name);
      if (node->type)
        out() << expr(node->type);
      if (node->value)
        out() << " = " << expr(node->value);
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