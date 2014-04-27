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

  class DefaultNodeVisitor: public DeclNodeVisitor
                          , public StmtNodeVisitor
                          , public ExprNodeVisitor
                          , public ConstDataNodeVisitor
                          , public TypeExprNodeVisitor
  {
  public:
    NodeVisitorCallback _cb;
    NodePtrVector       _parents;

    void pushParent(const NodePtr& p) {
      _parents.push_back(p);
    }
    void popParent() {
      _parents.pop_back();
    }


    void cb(const NodePtr& node) {
      if (_parents.size())
        _cb(_parents.back(), node);
      else
        _cb(NodePtr(), node);
    }

    DefaultNodeVisitor(NodeVisitorCallback nvc)
      : _cb(nvc)
    {}

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

    template <class T>
    void each(const std::vector< boost::shared_ptr<T> >& nodes) {
      typename std::vector< boost::shared_ptr<T> >::const_iterator it;
      for (it = nodes.begin(); it != nodes.end(); ++it) {
        accept(*it);
      }
    }

    virtual void acceptData(const ConstDataNodePtr& node)    { cb(node); pushParent(node); node->accept(this); popParent(); }
    virtual void acceptTypeExpr(const TypeExprNodePtr& node) { cb(node); pushParent(node); node->accept(this); popParent(); }
    virtual void acceptExpr(const ExprNodePtr& node)         { cb(node); pushParent(node); node->accept(this); popParent(); }
    virtual void acceptDecl(const DeclNodePtr& node)         { cb(node); pushParent(node); node->accept(this); popParent(); }
    virtual void acceptStmt(const StmtNodePtr& node)         { cb(node); pushParent(node); node->accept(this); popParent(); }

    void visitData(BoolConstDataNode *node) {
    }
    void visitData(IntConstDataNode *node) {
    }
    void visitData(FloatConstDataNode *node) {
    }
    void visitData(StringConstDataNode *node) {
    }
    void visitData(ListConstDataNode* node) {
      each(node->values);
    }
    void visitData(TupleConstDataNode* node) {
      each(node->values);
    }
    void visitData(DictConstDataNode* node) {
      //TODO: each(node->values);
    }

    void visitTypeExpr(SimpleTypeExprNode *node) {
    }
    void visitTypeExpr(ListTypeExprNode *node) {
      acceptTypeExpr(node->element);
    }
    void visitTypeExpr(MapTypeExprNode *node) {
      acceptTypeExpr(node->key);
      acceptTypeExpr(node->value);
    }
    void visitTypeExpr(TupleTypeExprNode *node) {
      each(node->elements);
    }

    void visitExpr(BinaryOpExprNode *node) {
      acceptExpr(node->n1);
      acceptExpr(node->n2);
    }
    void visitExpr(UnaryOpExprNode *node) {
      acceptExpr(node->n1);
    }
    void visitExpr(VarExprNode *node) {
    }
    void visitExpr(ConstDataExprNode* node) {
      acceptData(node->data);
    }

    void visitDecl(InterfaceDeclNode* node) {
      each(node->values);
    }
    void visitDecl(FnDeclNode* node) {
      each(node->args);
      if (node->ret)
        acceptTypeExpr(node->ret);
    }
    void visitDecl(EmitDeclNode* node) {
      each(node->args);
    }
    void visitDecl(PropDeclNode* node) {
      each(node->args);
    }
    void visitDecl(StructDeclNode* node) {
      each(node->fields);
    }
    void visitDecl(FieldDeclNode* node) {
      acceptTypeExpr(node->type);
    }
    void visitDecl(ConstDeclNode* node) {
    }

    void visitStmt(PackageNode* node) {
    }
    void visitStmt(ImportNode* node) {
    }
    void visitStmt(ObjectDefNode *node) {
      each(node->values);
    }
    void visitStmt(PropertyDefNode *node) {
      acceptData(node->data);
    }
    void visitStmt(AtNode* node) {
    }
    void visitStmt(VarDefNode* node) {
      acceptData(node->data);
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
