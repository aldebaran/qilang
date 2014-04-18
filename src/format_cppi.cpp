/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <iostream>
#include <qi/log.hpp>
#include <qilang/node.hpp>
#include <qilang/formatter.hpp>
#include <qi/os.hpp>
#include "formatter_p.hpp"
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "cpptype.hpp"

qiLogCategory("qigen.hppinterface");


namespace qilang {


class QiLangGenObjectDef : public NodeVisitor, public NodeFormatter {
public:
  int toclose;     //number of } to close (namespace)
  int noconstref;    //should we disable adding constref to types ?

  QiLangGenObjectDef()
    : toclose(0)
    , noconstref(0)
  {
  }

  virtual void accept(const NodePtr& node) { node->accept(this); }

  void formatHeader() {
    indent() << "/*" << std::endl;
    indent() << "** qiLang generated file. DO NOT EDIT" << std::endl;
    indent() << "*/" << std::endl;
    indent() << "#pragma once" << std::endl;
    boost::uuids::uuid u = boost::uuids::random_generator()();
    std::string uuid = boost::uuids::to_string(u);
    indent() << "#ifndef " << uuid << std::endl;
    indent() << "#define " << uuid << std::endl;
    indent() << std::endl;
    indent() << "#include <qitype/signal.hpp>" << std::endl;
    indent() << "#include <qitype/property.hpp>" << std::endl;
    indent() << std::endl;
  }

  void formatFooter() {
    for (int i = 0; i < toclose; ++i) {
      out() << "}" << std::endl;
    }
    out() << std::endl;
    indent() << "#endif" << std::endl;
  }

protected:
  void visit(PackageNode* node) {
    std::vector<std::string> ns = splitPkgName(node->name->name);
    for (int i = 0; i < ns.size(); ++i) {
      toclose++;
      indent() << "namespace " << ns.at(i) << " {" << std::endl;
    }
    out() << std::endl;
  }

  void visit(ImportNode* node) {
    throw std::runtime_error("unimplemented");
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
  void visit(TupleConstNode* node) {
    out() << "(" << "FAIL" << ")";
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
    throw std::runtime_error("unimplemented");
  }
  void visit(UnaryOpNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(VarNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(ExprNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(SimpleTypeNode *node) {
    out() << typeToCpp(node, noconstref==0);
  }
  void visit(ListTypeNode *node) {
    out() << typeToCpp(node, noconstref==0);
  }
  void visit(MapTypeNode *node) {
    out() << typeToCpp(node, noconstref==0);
  }
  void visit(TupleTypeNode *node) {
    out() << typeToCpp(node, noconstref==0);
  }

  //indented block
  void scopedDecl(const std::vector<qilang::NodePtr>& vec) {
    ScopedIndent _(_indent);
    for (unsigned int i = 0; i < vec.size(); ++i) {
      vec[i]->accept(this);
    }
  }
  void visit(ObjectNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(PropertyNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(AtNode* node) {
    throw std::runtime_error("unimplemented");
  }

  void visit(InterfaceDeclNode* node) {
    indent() << "class " << expr(node->name) << "Interface";
    if (node->inherits.size() > 0) {
      out() << ": ";
      for (int i = 0; i < node->inherits.size(); ++i) {
        out() << "public " << expr(node->inherits.at(i));
        if (i + 1 != node->inherits.size())
          out() << ", ";
      }
    }
    out() << " {" << std::endl;
    indent() << "public:" << std::endl;
    scopedDecl(node->values);
    indent() << "};" << std::endl << std::endl;
    indent() << "typedef qi::Object<" << expr(node->name) << "Interface> " << expr(node->name) << ";" << std::endl;
  }

  void visit(FnDeclNode* node) {
    if (node->ret)
      indent() << expr(node->ret) << " " << expr(node->name) << "(";
    else
      indent() << "void " << expr(node->name) << "(";

    for (unsigned int i = 0; i < node->args.size(); ++i) {
      out() << expr(node->args[i]);
      if (i+1 < node->args.size()) {
        out() << ", ";
      }
    }
    out() << ");" << std::endl;
  }

  void visit(InDeclNode* node) {
    indent() << "// slot" << std::endl;
    indent() << "void " << expr(node->name) << "(";
    for (unsigned int i = 0; i < node->args.size(); ++i) {
      out() << expr(node->args[i]);
      if (i+1 < node->args.size()) {
        out() << ", ";
      }
    }
    out() << ");" << std::endl;
  }
  void visit(OutDeclNode* node) {
    indent() << "qi::Signal<";
    for (unsigned int i = 0; i < node->args.size(); ++i) {
      out() << expr(node->args[i]);
      if (i+1 < node->args.size()) {
        out() << ", ";
      }
    }
    out() << "> " << expr(node->name) << ";" << std::endl;
  }
  void visit(PropDeclNode* node) {
    indent() << "qi::Property<";
    for (unsigned int i = 0; i < node->args.size(); ++i) {
      out() << expr(node->args[i]);
      if (i+1 < node->args.size()) {
        out() << ", ";
      }
    }
    out() << "> " << expr(node->name) << ";" << std::endl;
  }

  void visit(StructNode* node) {
    indent() << "struct " << expr(node->name) << " {" << std::endl;
    noconstref++;
    scopedDecl(node->values);
    noconstref--;
    indent() << "};" << std::endl << std::endl;
  }

  void visit(VarDefNode* node) {
    if (node->type)
      indent() << expr(node->type) << " " << expr(node->name);
    else
      indent() << "qi::AnyValue " << expr(node->name);
    if (node->value)
      out() << " = " << expr(node->value);
    out() << ";" << std::endl;
  }

  void visit(ConstDefNode* node) {
    indent() << "const ";
    if (node->type)
      out() << expr(node->type) << " " << expr(node->name);
    else
      out() << "qi::AnyValue " << expr(node->name);
    if (node->value)
      out() << " = " << expr(node->value);
    out() << ";" << std::endl;
  }

};


std::string genCppObjectInterface(const NodePtr& node) {
  return QiLangGenObjectDef().format(node);
}

std::string genCppObjectInterface(const NodePtrVector& nodes) {
  return QiLangGenObjectDef().format(nodes);
}

}
