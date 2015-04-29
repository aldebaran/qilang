/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#ifndef   	QILANG_VISITOR_HPP_
# define   	QILANG_VISITOR_HPP_

#include <qilang/node.hpp>
#include <boost/function.hpp>

namespace qilang {


  typedef boost::function<void (const NodePtr&, const NodePtr&)> NodeVisitorCallback;

  class DefaultNodeVisitor: public NodeVisitor
  {
    NodeVisitorCallback _cb;
    NodePtrVector       _parents;

  public:

    explicit DefaultNodeVisitor(NodeVisitorCallback nvc)
      : _cb(nvc)
    {}

    explicit DefaultNodeVisitor()
    {}

    virtual void doAccept(Node* node) { node->accept(this); }


    void pushParent(const NodePtr& p) {
      _parents.push_back(p);
    }
    void popParent() {
      _parents.pop_back();
    }

    void cb(const NodePtr& node) {
      if (!_cb)
        return;
      if (_parents.size())
        _cb(_parents.back(), node);
      else
        _cb(NodePtr(), node);
    }

    template <class T>
    void each(const std::vector< boost::shared_ptr<T> >& nodes) {
      typename std::vector< boost::shared_ptr<T> >::const_iterator it;
      for (it = nodes.begin(); it != nodes.end(); ++it) {
        acceptWithCb(*it);
      }
    }

    template <typename T>
    void acceptWithCb(const boost::shared_ptr<T>& node) {
      cb(node);
      pushParent(node);
      accept(node);
      popParent();
    }

    void visitData(BoolLiteralNode *node) {
    }
    void visitData(IntLiteralNode *node) {
    }
    void visitData(FloatLiteralNode *node) {
    }
    void visitData(StringLiteralNode *node) {
    }
    void visitData(ListLiteralNode* node) {
      each(node->values);
    }
    void visitData(TupleLiteralNode* node) {
      each(node->values);
    }
    void visitData(DictLiteralNode* node) {
      //TODO: each(node->values);
    }

    void visitTypeExpr(BuiltinTypeExprNode *node) {
    }
    void visitTypeExpr(CustomTypeExprNode *node) {
    }
    void visitTypeExpr(ListTypeExprNode *node) {
      acceptWithCb(node->element);
    }
    void visitTypeExpr(MapTypeExprNode *node) {
      acceptWithCb(node->key);
      acceptWithCb(node->value);
    }
    void visitTypeExpr(TupleTypeExprNode *node) {
      each(node->elements);
    }
    void visitTypeExpr(VarArgTypeExprNode *node) {
      acceptWithCb(node->element);
    }
    void visitTypeExpr(KeywordArgTypeExprNode *node) {
      acceptWithCb(node->value);
    }

    void visitExpr(BinaryOpExprNode *node) {
      acceptWithCb(node->left);
      acceptWithCb(node->right);
    }
    void visitExpr(UnaryOpExprNode *node) {
      acceptWithCb(node->expr);
    }
    void visitExpr(VarExprNode *node) {
    }
    void visitExpr(LiteralExprNode* node) {
      acceptWithCb(node->data);
    }
    void visitExpr(CallExprNode* node) {
      each(node->args);
    }

    void visitDecl(InterfaceDeclNode* node) {
      each(node->values);
    }
    void visitDecl(FnDeclNode* node) {
      each(node->args);
      if (node->ret)
        acceptWithCb(node->ret);
    }
    void visitDecl(ParamFieldDeclNode* node) {
      if (node->type)
        acceptWithCb(node->type);
    }

    void visitDecl(SigDeclNode* node) {
      each(node->args);
    }
    void visitDecl(PropDeclNode* node) {
      each(node->args);
    }
    void visitDecl(StructDeclNode* node) {
      each(node->decls);
    }
    void visitDecl(StructFieldDeclNode* node) {
      acceptWithCb(node->type);
    }
    void visitDecl(ConstDeclNode* node) {
    }
    void visitDecl(EnumDeclNode* node) {
      each(node->fields);
    }
    void visitDecl(EnumFieldDeclNode* node) {
    }
    void visitDecl(TypeDefDeclNode* node) {
      acceptWithCb(node->type);
    }

    void visitStmt(PackageNode* node) {
    }
    void visitStmt(ImportNode* node) {
    }
    void visitStmt(ObjectDefNode *node) {
      each(node->values);
    }
    void visitStmt(PropertyDefNode *node) {
      acceptWithCb(node->data);
    }
    void visitStmt(AtNode* node) {
    }
    void visitStmt(VarDefNode* node) {
      acceptWithCb(node->data);
    }

  };

  //! this will visit all given nodes and call the nvc callback on it
  template <class T>
  void visitNode(const std::vector< boost::shared_ptr<T> >& nodes, NodeVisitorCallback nvc)
  {
    DefaultNodeVisitor(nvc).each(nodes);
  }


  inline void findNodeTypeVisitor(const NodePtr& parent, const NodePtr& node, NodeType wanted, NodePtrVector& result) {
    if (node->type() == wanted)
      result.push_back(node);
  }

  inline void findNodeKindVisitor(const NodePtr& parent, const NodePtr& node, NodeKind wanted, NodePtrVector& result) {
    if (node->kind() == wanted)
      result.push_back(node);
  }

  inline NodePtrVector findNode(NodePtrVector nodes, NodeType type) {
    NodePtrVector result;
    qilang::visitNode(nodes, boost::bind<void>(&findNodeTypeVisitor, _1, _2, type, boost::ref(result)));
    return result;
  }

  inline NodePtrVector findNode(NodePtrVector nodes, NodeKind kind) {
    NodePtrVector result;
    qilang::visitNode(nodes, boost::bind<void>(&findNodeKindVisitor, _1, _2, kind, boost::ref(result)));
    return result;
  }


}

#endif	    /* !VISITOR_PP_ */
