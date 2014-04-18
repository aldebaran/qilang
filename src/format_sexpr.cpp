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

  class QiLangASTFormatter : public NodeFormatter, public NodeVisitor {
  protected:
    virtual void accept(const NodePtr& node) { node->accept(this); }

    void visit(PackageNode* node) {
      indent() << "(package " << expr(node->name) << ")" << std::endl;
    }

    void visit(ImportNode* node) {
      if (node->imported.size() == 0) {
        indent() << "(import " << expr(node->name) << ")" << std::endl;
      } else {
        indent() << "(from " << expr(node->name) << " (import ";
        for (int i = 0; i < node->imported.size(); ++i) {
          out() << expr(node->imported.at(i));
          if (i+1 < node->imported.size()) {
            out() << " ";
          }
        }
        out() << "))" << std::endl;
      }
    }

    const std::string &list(ConstExprNodePtrVector pv) {
      static const std::string ret;
      for (int i = 0; i < pv.size(); ++i) {
        expr(pv.at(i));
        if (i + 1 < pv.size())
          out() << " ";
      }
      return ret;
    }

    const std::string &dict(ConstExprNodePtrPairVector pv) {
      static const std::string ret;
      for (int i = 0; i < pv.size(); ++i) {
        out() << "(" << expr(pv.at(i).first) << " " << expr(pv.at(i).second) << ")";
        if (i + 1 < pv.size())
          out() << " ";
      }
      return ret;
    }

    void visit(IntConstNode *node) {
      out() << "(int " << node->value << ")";
    }
    void visit(FloatConstNode *node) {
      out() << "(float " << node->value << ")";
    }
    void visit(StringConstNode *node) {
      out() << "(string " << node->value << ")";
    }
    void visit(ListConstNode* node) {
      out() << "(list " << list(node->values) << ")";
    }
    void visit(TupleConstNode* node) {
      out() << "(tuple " << list(node->values) << ")";
    }
    void visit(DictConstNode* node) {
      out() << "(dict " << dict(node->values) << ")";
    }
    void visit(SymbolNode* node) {
      out() << "(symbol " << node->name << ")";
    }

    void visit(BinaryOpNode *node) {
      out() << "(" << BinaryOpCodeToString(node->op) << " " << expr(node->n1) << " " << expr(node->n2) << ")";
    }
    void visit(UnaryOpNode *node) {
      out() << "(" << UnaryOpCodeToString(node->op) << " " << expr(node->n1) << ")";
    }
    void visit(VarNode *node) {
       out() << "(var " << expr(node->value) << ")";
    }
    void visit(SimpleTypeNode *node) {
      out() << "(type " << node->value << ")";
    }
    void visit(ListTypeNode *node) {
      out() << "(listtype " << expr(node->element) << ")";
    }
    void visit(MapTypeNode *node) {
      out() << "(maptype " << expr(node->key) << " " << expr(node->value) << ")";
    }
    void visit(TupleTypeNode *node) {
      out() << "(tupletype ";
      for (int i = 0; i < node->elements.size(); ++i) {
        out() << expr(node->elements.at(i));
        if (i + 1 == node->elements.size())
          out() << " ";
      }
      out() << ")";
    }
    void visit(ExprNode *node) {
      out() << "(expr " << expr(node->value) << ")";
    }

    //indented block
    void scopedDecl(const std::vector<qilang::NodePtr>& vec) {
      ScopedIndent _(_indent);
      for (unsigned int i = 0; i < vec.size(); ++i) {
        vec[i]->accept(this);
      }
    }
    void visit(ObjectNode *node) {
      indent() << "(object " << expr(node->type) << " " << expr(node->id) << std::endl;
      scopedDecl(node->values);
      indent() << ")" << std::endl;
    }
    void visit(PropertyNode *node) {
      indent() << "(prop " << expr(node->var) << " " << expr(node->value) << ")" << std::endl;
    }
    void visit(AtNode* node) {
      indent() << "(at " << expr(node->sender) << " " << expr(node->receiver) << ")" << std::endl;
    }

    void visit(InterfaceDeclNode* node) {
      indent() << "(interface " << expr(node->name);
      if (node->inherits.size() > 0) {
        out() << "(inherit ";
        for (int i = 0; i < node->inherits.size(); ++i) {
          out() << expr(node->inherits.at(i));
          if (i + 1 != node->inherits.size())
            out() << " ";
        }
        out() << ")";
      }
      out() << std::endl;
      scopedDecl(node->values);
      indent() << ")" << std::endl;
    }

    void declParamList(const std::string &declname, const SymbolNodePtr& name, const TypeNodePtrVector& vec, const TypeNodePtr &ret = TypeNodePtr()) {
      out() << "(" << declname << " " << expr(name) << "(";
      for (unsigned int i = 0; i < vec.size(); ++i) {
        out() << expr(vec[i]);
        if (i+1 < vec.size()) {
          out() << " ";
        }
      }
      out() << ")";
      if (ret)
        out() << " " << expr(ret);
      out() << ")" << std::endl;
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
      indent() << "(struct " << expr(node->name) << std::endl;
      scopedDecl(node->values);
      indent() << ")" << std::endl;
    }

    void visit(VarDefNode* node) {
      indent() << "(defvar " << expr(node->name);
      if (node->type)
        out() << " " << expr(node->type);
      if (node->value)
        out() << " " << expr(node->value);
      out() << ")" << std::endl;
    }

    void visit(ConstDefNode* node) {
      indent() << "(defconst " << expr(node->name);
      if (node->type)
        out() << " " << expr(node->type);
      if (node->value)
        out() << " " << expr(node->value);
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
