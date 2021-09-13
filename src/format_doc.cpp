/*
** Author(s):
**  - Philippe Daouadi <pdaouadi@aldebaran.com>
**
** Copyright (C) 2015 Aldebaran Robotics
*/

#include <iostream>
#include <ka/scoped.hpp>
#include <qi/log.hpp>
#include <qilang/node.hpp>
#include <qilang/formatter.hpp>
#include <qilang/packagemanager.hpp>
#include <qilang/docparser.hpp>
#include <qi/os.hpp>
#include "formatter_p.hpp"
#include <boost/algorithm/string/replace.hpp>
#include <stack>
#include "cpptype.hpp"

qiLogCategory("qigen.hppinterface");


namespace qilang {

static std::string builtinTypeToString(BuiltinType type) {
  switch (type) {
  case BuiltinType_Nothing:
    return "nothing";
  case BuiltinType_Bool:
    return "bool";
  case BuiltinType_Char:
    return "char";
  case BuiltinType_Int:
    return "int";
  case BuiltinType_UInt:
    return "uint";
  case BuiltinType_Int8:
    return "int8";
  case BuiltinType_UInt8:
    return "uint8";
  case BuiltinType_Int16:
    return "int16";
  case BuiltinType_UInt16:
    return "uint16";
  case BuiltinType_Int32:
    return "int32";
  case BuiltinType_UInt32:
    return "uint32";
  case BuiltinType_Int64:
    return "int64";
  case BuiltinType_UInt64:
    return "uint64";
  case BuiltinType_Float:
    return "float";
  case BuiltinType_Float32:
    return "float32";
  case BuiltinType_Float64:
    return "float64";
  case BuiltinType_NanoSeconds:
    return "nsec";
  case BuiltinType_MicroSeconds:
    return "usec";
  case BuiltinType_MilliSeconds:
    return "msec";
  case BuiltinType_Seconds:
    return "sec";
  case BuiltinType_Minutes:
    return "min";
  case BuiltinType_Hours:
    return "hour";
  case BuiltinType_QiTimePoint:
    return "qitimepoint";
  case BuiltinType_SteadyTimePoint:
    return "steadytimepoint";
  case BuiltinType_SystemTimePoint:
    return "systemtimepoint";
  case BuiltinType_String:
    return "str";
  case BuiltinType_Value:
    return "any";
  case BuiltinType_Object:
    return "obj";
  }
  throw std::runtime_error("unreachable code");
}

class QiLangGenDoc: public NodeFormatter<>
{
public:
  QiLangGenDoc() {
    first.push(true);
  }

  std::stack<bool> first;
  std::unique_ptr<Doc> curDoc;

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
    out() << "\"type\" : \"interface\", ";
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
    accept(node->effectiveType());
    out() << "\"";
    if (curDoc) {
      Doc::Parameters::const_iterator doc = curDoc->parameters.find(node->names.at(0));
      if (doc != curDoc->parameters.end())
        out() << ", \"description\" : \"" << doc->second << "\"";
      else
        qiLogWarning() << "undocumented parameter: " << node->names.at(0);
    }
    out() << " }";
  }

  void visitDecl(FnDeclNode* node) {
    putComma();

    if (!node->comment().empty())
    {
      Doc doc = parseDoc(node->comment());

      curDoc.reset(new Doc(doc));
    }
    ka::scoped([this] { curDoc.reset(); });

    out() << "\"" << node->name << "\" : {";
    out() << "\"type\" : \"method\", ";
    if (curDoc && curDoc->brief)
      out() << "\"brief\" : \"" << *curDoc->brief << "\", ";
    if (curDoc && curDoc->description)
      out() << "\"description\" : \"" << *curDoc->brief << "\", ";
    if (curDoc && curDoc->return_)
      out() << "\"returnDescription\" : \"" << *curDoc->return_ << "\", ";
    out() << "\"return\" : \"";
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

  void visitDecl(SigDeclNode* node) {
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
  virtual void visitTypeExpr(BuiltinTypeExprNode* node) {
    out() << builtinTypeToString(node->builtinType);
  }
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

  void visitStmt(PropertyDefNode *node) {
    throw std::runtime_error("unimplemented");
  }

  void visitStmt(VarDefNode* node) {
  }

};

std::string genDoc(const NodePtr& node) {
  return "{" + QiLangGenDoc().format(node) + "}";
}

std::string genDoc(const NodePtrVector& node) {
  return "{" + QiLangGenDoc().format(node) + "}";
}

}
