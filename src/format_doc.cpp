/*
** Author(s):
**  - Philippe Daouadi <pdaouadi@aldebaran.com>
**
** Copyright (C) 2015 Aldebaran Robotics
*/

#include <iostream>
#include <qi/log.hpp>
#include <qilang/node.hpp>
#include <qilang/formatter.hpp>
#include <qilang/packagemanager.hpp>
#include <qi/os.hpp>
#include "formatter_p.hpp"
#include <boost/algorithm/string/replace.hpp>
#include <boost/lambda/lambda.hpp>
#include <stack>
#include "cpptype.hpp"

qiLogCategory("qigen.hppinterface");


namespace qilang {

class QiLangGenDoc: public NodeFormatter
{
public:
  QiLangGenDoc() {
    first.push(true);
  }

  std::stack<bool> first;

  void putComma() {
    if (first.top())
      first.top() = false;
    else
      out() << ", ";
  }

  struct ScopePush {
    ScopePush(std::stack<bool>& stack)
      : stack(stack)
    {
      stack.push(true);
    }

    ~ScopePush() {
      stack.pop();
    }

    std::stack<bool>& stack;
  };

  virtual void doAccept(Node* node) { node->accept(this); }

  void visitDecl(InterfaceDeclNode* node) {
    putComma();

    out() << "\"" << node->name << "\" : {";
    out() << "\"type\" : \"interface\",";
    out() << "\"inherits\" : [";

    for (unsigned int i = 0; i < node->inherits.size(); ++i) {
      out() << "\"" << node->inherits.at(i) << "\"";
      if (i + 1 != node->inherits.size())
        out() << ", ";
    }
    out() << "], \"members\" : {";

    {
      ScopePush scope(first);
      for (unsigned int i = 0; i < node->values.size(); ++i) {
        accept(node->values.at(i));
      }
    }

    out() << "}}";
  }

  void visitDecl(ParamFieldDeclNode* node) {
    putComma();

    out() << "\"" << node->names.at(0) << "\" : ";
    out() << "{ \"type\": \"";
    out() << "TODO";
    out() << "\" }";
  }

  void visitDecl(FnDeclNode* node) {
    putComma();

    out() << "\"" << node->name << "\" : {";
    out() << "\"type\" : \"method\", ";
    out() << "\"return\" : \"";
    out() << "TODO";
    accept(node->effectiveRet());
    out() << "\", ";
    out() << "\"parameters\" : {";
    {
      ScopePush scope(first);
      for (unsigned i = 0; i < node->args.size(); ++i) {
        visitDecl(node->args[i].get());
      }
    }
    out() << "}}";
  }

  void visitDecl(EmitDeclNode* node) {
    putComma();

    out() << "\"" << node->name << "\" : {";
    out() << "\"type\" : \"signal\", ";
    out() << "\"parameters\" : {";
    {
      ScopePush scope(first);
      for (unsigned i = 0; i < node->args.size(); ++i) {
        visitDecl(node->args[i].get());
      }
    }
    out() << "}}";
  }
  void visitDecl(PropDeclNode* node) {
    putComma();

    out() << "\"type\" : \"property\", ";
    out() << "\"parameters\" : {";
    {
      ScopePush scope(first);
      for (unsigned i = 0; i < node->args.size(); ++i) {
        visitDecl(node->args[i].get());
      }
    }
    out() << "}}";
  }

  void visitDecl(StructDeclNode* node) {
  }

  void visitDecl(ConstDeclNode* node) {
  }

  void visitDecl(StructFieldDeclNode* node) {
  }
  void visitDecl(EnumDeclNode* node) {
  }
  void visitDecl(EnumFieldDeclNode* node) {
  }
  void visitDecl(TypeDefDeclNode* node) {
  }

  virtual void visitExpr(BinaryOpExprNode* node) {}
  virtual void visitExpr(UnaryOpExprNode* node) {}
  virtual void visitExpr(VarExprNode* node) {}
  virtual void visitExpr(LiteralExprNode* node) {}
  virtual void visitExpr(CallExprNode* node) {}
  virtual void visitData(BoolLiteralNode* node) {}
  virtual void visitData(IntLiteralNode* node) {}
  virtual void visitData(FloatLiteralNode* node) {}
  virtual void visitData(StringLiteralNode* node) {}
  virtual void visitData(TupleLiteralNode* node) {}
  virtual void visitData(ListLiteralNode* node) {}
  virtual void visitData(DictLiteralNode* node) {}
  virtual void visitTypeExpr(BuiltinTypeExprNode* node) {}
  virtual void visitTypeExpr(CustomTypeExprNode* node) {}
  virtual void visitTypeExpr(ListTypeExprNode* node) {}
  virtual void visitTypeExpr(MapTypeExprNode* node) {}
  virtual void visitTypeExpr(TupleTypeExprNode* node) {}
  virtual void visitTypeExpr(VarArgTypeExprNode *node) {}
  virtual void visitTypeExpr(KeywordArgTypeExprNode *node) {}

protected:
  void visitStmt(PackageNode* node) {
  }

  void visitStmt(ImportNode* node) {
  }
  void visitStmt(ObjectDefNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visitStmt(PropertyDefNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visitStmt(AtNode* node) {
    throw std::runtime_error("unimplemented");
  }
  void visitStmt(VarDefNode* node) {
  }
  void visitStmt(CommentNode* node) {
  }

};

std::string genDoc(const NodePtr& node) {
  return "{" + QiLangGenDoc().format(node) + "}";
}

std::string genDoc(const NodePtrVector& node) {
  return "{" + QiLangGenDoc().format(node) + "}";
}

}
