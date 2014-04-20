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


  class QiLangASTFormatter : public FileFormatter,
                             public StmtNodeVisitor,
                             public DeclNodeVisitor,
                             public ConstDataNodeFormatter,
                             public ExprNodeFormatter,
                             public TypeExprNodeFormatter
  {
  protected:
    //virtual void accept(const NodePtr& node)          { node->accept((NodeVisitor*)this); }
    virtual void accept(const ExprNodePtr& node)      { node->accept((ExprNodeVisitor*)this); }
    virtual void accept(const ConstDataNodePtr& node) { node->accept((ConstDataNodeVisitor*)this); }
    virtual void accept(const TypeExprNodePtr& node)  { node->accept((TypeExprNodeVisitor*)this); }

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

    const std::string &list(ConstDataNodePtrVector pv) {
      static const std::string ret;
      for (int i = 0; i < pv.size(); ++i) {
        cdata(pv.at(i));
        if (i + 1 < pv.size())
          out() << " ";
      }
      return ret;
    }

    const std::string &dict(ConstDataNodePtrPairVector pv) {
      static const std::string ret;
      for (int i = 0; i < pv.size(); ++i) {
        out() << "(" << cdata(pv.at(i).first) << " " << cdata(pv.at(i).second) << ")";
        if (i + 1 < pv.size())
          out() << " ";
      }
      return ret;
    }
    void visit(BoolConstDataNode *node) {
      out() << "(bool " << node->value << ")";
    }
    void visit(IntConstDataNode *node) {
      out() << "(int " << node->value << ")";
    }
    void visit(FloatConstDataNode *node) {
      out() << "(float " << node->value << ")";
    }
    void visit(StringConstDataNode *node) {
      out() << "(string " << node->value << ")";
    }
    void visit(ListConstDataNode* node) {
      out() << "(list " << list(node->values) << ")";
    }
    void visit(TupleConstDataNode* node) {
      out() << "(tuple " << list(node->values) << ")";
    }
    void visit(DictConstDataNode* node) {
      out() << "(dict " << dict(node->values) << ")";
    }
    void visit(SimpleTypeExprNode *node) {
      out() << "(type " << node->value << ")";
    }
    void visit(ListTypeExprNode *node) {
      out() << "(listtype " << type(node->element) << ")";
    }
    void visit(MapTypeExprNode *node) {
      out() << "(maptype " << type(node->key) << " " << type(node->value) << ")";
    }
    void visit(TupleTypeExprNode *node) {
      out() << "(tupletype ";
      for (int i = 0; i < node->elements.size(); ++i) {
        out() << type(node->elements.at(i));
        if (i + 1 == node->elements.size())
          out() << " ";
      }
      out() << ")";
    }

    void visit(BinaryOpExprNode *node) {
      out() << "(" << BinaryOpCodeToString(node->op) << " " << expr(node->n1) << " " << expr(node->n2) << ")";
    }
    void visit(UnaryOpExprNode *node) {
      out() << "(" << UnaryOpCodeToString(node->op) << " " << expr(node->n1) << ")";
    }
    void visit(VarExprNode *node) {
       out() << "(var " << node->value << ")";
    }

    //indented block
    void scopedDecl(const std::vector<qilang::NodePtr>& vec) {
      ScopedIndent _(_indent);
      for (unsigned int i = 0; i < vec.size(); ++i) {
        vec[i]->accept(this);
      }
    }
    void visit(ObjectDefNode *node) {
      indent() << "(object " << type(node->type) << " " << cdata(node->name) << std::endl;
      scopedDecl(node->values);
      indent() << ")" << std::endl;
    }
    void visit(PropertyDefNode *node) {
      indent() << "(prop " << node->name << " " << cdata(node->value) << ")" << std::endl;
    }
    void visit(AtNode* node) {
      indent() << "(at " << node->sender << " " << node->receiver << ")" << std::endl;
    }

    void visit(InterfaceDeclNode* node) {
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
        out() << type(vec[i]);
        if (i+1 < vec.size()) {
          out() << " ";
        }
      }
      out() << ")";
      if (ret)
        out() << " " << type(ret);
      out() << ")" << std::endl;
    }

    void visit(FnDeclNode* node) {
      declParamList("fn", node->name, node->args, node->ret);
    }
    void visit(EmitDeclNode* node) {
      declParamList("out", node->name, node->args);
    }
    void visit(PropDeclNode* node) {
      declParamList("prop", node->name, node->args);
    }

    void visit(StructDeclNode* node) {
      indent() << "(struct " << node->name << std::endl;
      scopedDecl(node->values);
      indent() << ")" << std::endl;
    }

    void visit(VarDefNode* node) {
      indent() << "(defvar " << node->name;
      if (node->type)
        out() << " " << type(node->type);
      if (node->data)
        out() << " " << cdata(node->data);
      out() << ")" << std::endl;
    }

    void visit(ConstDeclNode* node) {
      indent() << "(defconst " << expr(node->name);
      if (node->type)
        out() << " " << expr(node->type);
      if (node->data)
        out() << " " << expr(node->data);
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
