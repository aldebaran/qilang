/*
** Author(s):
**  - Cedric GESTES  <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <sstream>
#include <stdexcept>
#include <qilang/node.hpp>

namespace qilang {

  class Formatter {
  public:
    class ScopedIndent {
      int &_indent;
      int _add;
    public:
      ScopedIndent(int& indent, int add = 2)
        : _indent(indent)
        , _add(add)
      {
        _indent += _add;
      }
      ~ScopedIndent() { _indent -= _add; }
    };

  public:
    std::stringstream &indent(int changes = 0) {
      _indent += changes;
      if (_indent < 0)
        _indent = 0;
      for (int i = 0; i < _indent; ++i) {
        ss.put(' ');
      }
      return ss;
    }

    std::stringstream &out() {
      return ss;
    }

  public:
    int               _indent;
  private:
    std::stringstream ss;

  };

  class QiLangFormatter : public Formatter, public NodeVisitor {
  public:
    std::string format(NodePtr node) {
      node->accept(this);
      return out().str();
    }

  protected:
    //indented block
    void scopedDecl(const std::vector<qilang::NodePtr>& vec) {
      ScopedIndent _(_indent);
      for (unsigned int i = 0; i < vec.size(); ++i) {
        indent() << ::qilang::format(vec[i]);
      }
    }

    void visit(IntNode *node) {
      out() << node->value;
    }
    void visit(FloatNode *node) {
      out() << node->value;
    }
    void visit(StringNode *node) {
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
    void visit(ObjectPropertyNode *node) {
      indent() << "prop " << node->var << " " << ::qilang::format(node->value) << std::endl;
    }
    void visit(AtNode* node) {
      indent() << "at " << node->sender << " " << node->receiver << std::endl;
    }

    void visit(InterfaceNode* node) {
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

  std::string format(NodePtr node) {
    if (!node)
      throw std::runtime_error("Invalid Node");
    return QiLangFormatter().format(node);
  }



  class QiLangASTFormatter : public Formatter, public NodeVisitor {
  public:
    std::string format(NodePtr node) {
      node->accept(this);
      return out().str();
    }

  protected:
    void visit(IntNode *node) {
      out() << "(int " << node->value << ")";
    }
    void visit(FloatNode *node) {
      out() << "(float " << node->value << ")";
    }
    void visit(StringNode *node) {
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
        indent() << ::qilang::formatAST(vec[i]);
      }
    }
    void visit(ObjectNode *node) {
      out() << "(object " << node->type << " " << node->id << std::endl;
      scopedDecl(node->values);
      out() << ")";
    }
    void visit(ObjectPropertyNode *node) {
      indent() << "(prop " << node->var << " " << formatAST(node->value) << ")" << std::endl;
    }
    void visit(AtNode* node) {
      indent() << "(at " << node->sender << " " << node->receiver << ")" << std::endl;
    }

    void visit(InterfaceNode* node) {
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


  std::string formatAST(NodePtr node) {
    if (!node)
      throw std::runtime_error("Invalid Node");
    return QiLangASTFormatter().format(node);
  }




}
