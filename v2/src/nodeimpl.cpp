/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <qilang2/node.hpp>

namespace qilang2 {

  class NodeImpl : public virtual NodeInterface {
  public:
    NodeImpl(NodeKind kind, NodeType type, const Location& loc)
      : _kind(kind)
      , _type(type)
      , _loc(loc)
    {}

    virtual NodeKind kind() { return _kind; }
    virtual NodeType type() { return _type; }
    virtual Location loc()  { return _loc; }

    NodeKind _kind;
    NodeType _type;
    Location _loc;
  };

  class ExprNodeImpl : public virtual ExprNodeInterface, public NodeImpl {
  protected:
    explicit ExprNodeImpl(NodeKind kind, NodeType type, const Location& loc)
      : NodeImpl(kind, type, loc)
    {}
    explicit ExprNodeImpl(NodeType type, const Location& loc)
      : NodeImpl(NodeKind_Expr, type, loc)
    {}
  };

  class VarExprNodeImpl: public virtual VarExprNodeInterface, public ExprNodeImpl {
  public:
    VarExprNodeImpl(const std::string& name, const qilang2::Location& loc)
      : ExprNodeImpl(NodeType_VarExpr, loc)
      , _name(name)
    {}

    std::string _name;

    std::string name() { return _name; }
  };

  VarExprNode newVarExprNode(const std::string& name, const qilang2::Location& loc)
  {
    return VarExprNode(new VarExprNodeImpl(name, loc));
  }

};
