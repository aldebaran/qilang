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
  public:
    std::string format(const NodePtrVector& node) {
      for (int i = 0; i < node.size(); ++i) {
        if (!node.at(i))
          throw std::runtime_error("Invalid Node");
        node.at(i)->accept(this);
      }
      return out().str();
    }

    std::string format(const NodePtr& node) {
      if (!node)
        throw std::runtime_error("Invalid Node");
      node->accept(this);
      return out().str();
    }


  protected:
    //indented block
    void scopedDecl(const qilang::NodePtrVector& vec) {
      ScopedIndent _(_indent);
      for (unsigned int i = 0; i < vec.size(); ++i) {
        vec[i]->accept(this);
      }
    }
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

    void visit(IntConstNode *node) {
      out() << node->value;
    }
    void visit(FloatConstNode *node) {
      out() << node->value;
    }
    void visit(StringConstNode *node) {
      out() << node->value;
    }
    void visit(BinaryOpNode *node) {
      out() << ::qilang::format(node->n1) << " " << BinaryOpCodeToString(node->op) << " " << ::qilang::format(node->n2);
    }
    void visit(UnaryOpNode *node) {
      out() << UnaryOpCodeToString(node->op) << ::qilang::format(node->n1);
    }
    void visit(VarNode *node) {
      indent() << "(var " << node->value << ")";
    }
    void visit(ExprNode *node) {
      out() << "(" << ::qilang::format(node->value) << ")" << std::endl;
    }

    void visit(ObjectNode *node) {
      indent() << "object " << node->type << " " << node->id << std::endl;
      scopedDecl(node->values);
      indent() << "end" << std::endl;
    }
    void visit(PropertyNode *node) {
      indent() << "prop " << node->var << " " << ::qilang::format(node->value) << std::endl;
    }
    void visit(AtNode* node) {
      indent() << "at " << node->sender << " " << node->receiver << std::endl;
    }
    void visit(InterfaceDeclNode* node) {
      indent() << "interface " << node->name << std::endl;
      scopedDecl(node->values);
      indent() << "end" << std::endl;
    }

    // a, ..., z
    template <class T>
    void declParamList(const std::string &declname, const std::string& name, const std::vector<T>& vec, const std::string &ret = "") {
      indent() << declname << " " << name << "(";
      for (unsigned int i = 0; i < vec.size(); ++i) {
        out() << vec[i];
        if (i+1 < vec.size()) {
          out() << ", ";
        }
      }
      out() << ")";
      if (!ret.empty())
        out() << " " << ret;
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

  };








  class QiLangASTFormatter : public NodeFormatter, public NodeVisitor {
  public:
    std::string format(const NodePtrVector& node) {
      for (int i = 0; i < node.size(); ++i) {
        if (!node.at(i))
          throw std::runtime_error("Invalid Node");
        node.at(i)->accept(this);
      }
      return out().str();
    }

    std::string format(const NodePtr& node) {
      if (!node)
        throw std::runtime_error("Invalid Node");
      node->accept(this);
      return out().str();
    }

  protected:
    void visit(PackageNode* node) {
      indent() << "(package " << node->name << ")" << std::endl;
    }

    void visit(ImportNode* node) {
      if (node->imported.size() == 0) {
        indent() << "(import " << node->name << ")" << std::endl;
      } else {
        indent() << "(from " << node->name << " (import ";
        for (int i = 0; i < node->imported.size(); ++i) {
          out() << node->imported.at(i);
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
    void visit(BinaryOpNode *node) {
      out() << "(" << BinaryOpCodeToString(node->op) << " " << formatAST(node->n1) << " " << formatAST(node->n2) << ")";
    }
    void visit(UnaryOpNode *node) {
      out() << "(" << UnaryOpCodeToString(node->op) << " " << formatAST(node->n1) << ")";
    }
    void visit(VarNode *node) {
       out() << "(var " << node->value << ")";
    }
    void visit(ExprNode *node) {
      out() << "(expr " << formatAST(node->value) << ")";
    }

    //indented block
    void scopedDecl(const std::vector<qilang::NodePtr>& vec) {
      ScopedIndent _(_indent);
      for (unsigned int i = 0; i < vec.size(); ++i) {
        vec[i]->accept(this);
      }
    }
    void visit(ObjectNode *node) {
      indent() << "(object " << node->type << " " << node->id << std::endl;
      scopedDecl(node->values);
      indent() << ")" << std::endl;
    }
    void visit(PropertyNode *node) {
      indent() << "(prop " << node->var << " " << formatAST(node->value) << ")" << std::endl;
    }
    void visit(AtNode* node) {
      indent() << "(at " << node->sender << " " << node->receiver << ")" << std::endl;
    }

    void visit(InterfaceDeclNode* node) {
      indent() << "(interface " << node->name << std::endl;
      scopedDecl(node->values);
      indent() << ")" << std::endl;
    }

    template <class T>
    void declParamList(const std::string &declname, const std::string& name, const std::vector<T>& vec, const std::string &ret = "") {
      out() << "(" << declname << " " << name << "(";
      for (unsigned int i = 0; i < vec.size(); ++i) {
        out() << vec[i];
        if (i+1 < vec.size()) {
          out() << " ";
        }
      }
      out() << ")";
      if (!ret.empty())
        out() << " " << ret;
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
