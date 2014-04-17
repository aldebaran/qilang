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
#include <boost/static_assert.hpp>

namespace std {
  //only to avoid mistake... (shared_ptr are displayed as pointer by default...)
  //this function will generate an error instead
  template <class T>
  std::ostream& operator<<(std::ostream&o, const boost::shared_ptr<T>& node) {
    o << *node.get();
  }
}

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
      indent() << "package " << node->name->str() << std::endl;
    }

    void visit(ImportNode* node) {
      if (node->imported.size() == 0)
        indent() << "import " << node->name->str();
      else {
        indent() << "from " << node->name->str() << " import ";
        for (unsigned int i = 0; i < node->imported.size(); ++i) {
          out() << node->imported.at(i)->str();
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
      out() << "[" << "FAIL" << "]";
    }
    void visit(DictConstNode* node) {
      out() << "{" << "FAIL" << "}";
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


    // #############
    // STATEMENT
    // #############
    void visit(VarNode *node) {
      indent() << "(var " << node->value->str() << ")";
    }
    void visit(ObjectNode *node) {
      indent() << "object " << node->type->str() << " " << expr(node->id) << std::endl;
      scopedDecl(node->values);
      indent() << "end" << std::endl << std::endl;
    }
    void visit(PropertyNode *node) {
      indent() << "prop " << node->var->str() << " " << expr(node->value) << std::endl;
    }
    void visit(AtNode* node) {
      indent() << "at " << expr(node->sender) << " " << expr(node->receiver) << std::endl;
    }
    void visit(InterfaceDeclNode* node) {
      indent() << "interface " << node->name->str() << std::endl;
      scopedDecl(node->values);
      indent() << "end" << std::endl << std::endl;
    }

    // a, ..., z
    void declParamList(const std::string& declname, const SymbolNodePtr& name, const SymbolNodePtrVector& vec, const SymbolNodePtr& ret = SymbolNodePtr()) {
      indent() << declname << " " << name->str() << "(";
      for (unsigned int i = 0; i < vec.size(); ++i) {
        out() << vec[i]->str();
        if (i+1 < vec.size()) {
          out() << ", ";
        }
      }
      out() << ")";
      if (!ret)
        out() << " " << ret->str();
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
      indent() << "struct " << node->name->str() << std::endl;
      scopedDecl(node->values);
      indent() << "end" << std::endl << std::endl;
    }
    void visit(VarDefNode* node) {
      indent() << node->name->str();
      if (node->type)
        out() << " " << expr(node->type);
      if (node->value)
        out() << " = " << expr(node->value);
      out() << std::endl;
    }
    void visit(ConstDefNode* node) {
      indent() << "const " << node->name->str();
      if (node->type)
        out() << expr(node->type);
      if (node->value)
        out() << " = " << expr(node->value);
      out() << std::endl;
    }

  };








  class QiLangASTFormatter : public NodeFormatter, public NodeVisitor {
  protected:
    virtual void accept(const NodePtr& node) { node->accept(this); }

    void visit(PackageNode* node) {
      indent() << "(package " << node->name->str() << ")" << std::endl;
    }

    void visit(ImportNode* node) {
      if (node->imported.size() == 0) {
        indent() << "(import " << node->name->str() << ")" << std::endl;
      } else {
        indent() << "(from " << node->name->str() << " (import ";
        for (int i = 0; i < node->imported.size(); ++i) {
          out() << node->imported.at(i)->str();
          if (i+1 < node->imported.size()) {
            out() << " ";
          }
        }
        out() << "))" << std::endl;
      }
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
      out() << "(list " << "FAIL" << ")";
    }


    void visit(DictConstNode* node) {
      out() << "(dict " << "FAIL" << ")";
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
       out() << "(var " << node->value->str() << ")";
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
      indent() << "(object " << node->type->str() << " " << expr(node->id) << std::endl;
      scopedDecl(node->values);
      indent() << ")" << std::endl;
    }
    void visit(PropertyNode *node) {
      indent() << "(prop " << node->var->str() << " " << expr(node->value) << ")" << std::endl;
    }
    void visit(AtNode* node) {
      indent() << "(at " << expr(node->sender) << " " << expr(node->receiver) << ")" << std::endl;
    }

    void visit(InterfaceDeclNode* node) {
      indent() << "(interface " << node->name->str() << std::endl;
      scopedDecl(node->values);
      indent() << ")" << std::endl;
    }

    void declParamList(const std::string &declname, const SymbolNodePtr& name, const SymbolNodePtrVector& vec, const SymbolNodePtr &ret = SymbolNodePtr()) {
      out() << "(" << declname << " " << name->str() << "(";
      for (unsigned int i = 0; i < vec.size(); ++i) {
        out() << vec[i]->str();
        if (i+1 < vec.size()) {
          out() << " ";
        }
      }
      out() << ")";
      if (ret)
        out() << " " << ret->str();
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
      indent() << "(struct " << node->name->str() << std::endl;
      scopedDecl(node->values);
      indent() << ")" << std::endl;
    }

    void visit(VarDefNode* node) {
      indent() << "(defvar " << node->name->str() << " ";
      if (node->type)
        out() << expr(node->type) << " ";
      if (node->value)
        out() << expr(node->value) << ")";
      out() << std::endl;
    }

    void visit(ConstDefNode* node) {
      indent() << "(defconst " << node->name->str() << " ";
      if (node->type)
        out() << expr(node->type) << " ";
      if (node->value)
        out() << expr(node->value) << ")";
      out() << std::endl;
    }


  };

  std::string formatAST(const NodePtr& node) {
    return QiLangASTFormatter().format(node);
  }

  std::string format(const NodePtr& node) {
    return QiLangFormatter().format(node);
  }

  std::string formatAST(const NodePtrVector& node) {
    return QiLangASTFormatter().format(node);
  }

  std::string format(const NodePtrVector& node) {
    return QiLangFormatter().format(node);
  }



}
