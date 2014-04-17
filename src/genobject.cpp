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
#include <boost/algorithm/string.hpp>

qiLogCategory("qigen.hppinterface");

namespace std {
  //only to avoid mistake... (shared_ptr are displayed as pointer by default...)
  //this function will generate an error instead
  template <class T>
  std::ostream& operator<<(std::ostream&o, const boost::shared_ptr<T>& node) {
    o << *node.get();
  }
}

static std::string typeToCpp(const std::string& type) {
  const char* pod[] = { "bool", "char", "int",
                        "int8", "uint8",
                        "int16", "uint16",
                        "int32", "uint32",
                        "int64", "uint64",
                        "float32", "float64",
                        0 };
  int i = 0;
  while (pod[i]) {
    if (type == pod[i])
      return type;
    ++i;
  }
  return "const " + type + "&";
}

static std::vector<std::string> splitPkgName(const std::string& name) {
  std::vector<std::string> ret;

  boost::split(ret, name, boost::is_any_of("."));
  return ret;
}

namespace qilang {


class QiLangGenObjectDef : public NodeVisitor, public NodeFormatter {
public:
  int toclose;

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
  void visit(TypeNode *node) {
    out() << typeToCpp(node->value);
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
    indent() << "class " << expr(node->name) << "Interface {" << std::endl;
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
    scopedDecl(node->values);
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

//Generate Type Registration Information
class QiLangGenObjectDecl : public NodeVisitor, public NodeFormatter {
public:
  int toclose;
  int id;
  std::string currentParent;

  virtual void accept(const NodePtr& node) { node->accept(this); }

  void formatHeader() {
    indent() << "/*" << std::endl;
    indent() << "** qiLang generated file. DO NOT EDIT" << std::endl;
    indent() << "*/" << std::endl;
  }

  void formatFooter() {
    for (int i = 0; i < toclose; ++i) {
      out() << "}" << std::endl;
    }
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
  void visit(TypeNode *node) {
    out() << typeToCpp(node->value);
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
    int current = id;
    id++;
    currentParent = node->name->name + "Interface";
    indent() << "static int initType" << current << "() {" << std::endl;
    {
      ScopedIndent _(_indent);
      indent() << "qi::ObjectTypeBuilder<" << currentParent << "> builder;" << std::endl;
      for (unsigned int i = 0; i < node->values.size(); ++i) {
        accept(node->values.at(i));
      }
      currentParent = "";
    }
    indent() << "}" << std::endl;
    indent() << "static int myinittype" << current << " = initType" << current << "()" << std::endl;
    indent() << std::endl;
  }
  void visit(FnDeclNode* node) {
    indent() << "builder.advertiseMethod(\"" << expr(node->name) << "\", &" << currentParent << "::" << expr(node->name);
    out() << ");" << std::endl;
  }
  void visit(InDeclNode* node) {
    indent() << "builder.advertiseMethod(\"" << expr(node->name) << "\", &" << currentParent << "::" << expr(node->name);
    out() << ");" << std::endl;
  }
  void visit(OutDeclNode* node) {
    indent() << "builder.advertiseSignal(\"" << expr(node->name) << "\", &" << currentParent << "::" << expr(node->name);
    out() << ");" << std::endl;
  }
  void visit(PropDeclNode* node) {
    indent() << "builder.advertiseProperty(\"" << expr(node->name) << "\", &" << currentParent << "::" << expr(node->name);
    out() << ");" << std::endl;
  }
  void visit(StructNode* node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(VarDefNode* node) {
    throw std::runtime_error("unimplemented");
  }
  void visit(ConstDefNode* node) {
    throw std::runtime_error("unimplemented");
  }

};



std::string genCppObjectInterface(const NodePtr& node) {
  return QiLangGenObjectDef().format(node);
}

std::string genCppObjectInterface(const NodePtrVector& nodes) {
  return QiLangGenObjectDef().format(nodes);
}

std::string genCppObjectRegistration(const NodePtr& node) {
  return QiLangGenObjectDecl().format(node);
}

std::string genCppObjectRegistration(const NodePtrVector& nodes) {
  return QiLangGenObjectDecl().format(nodes);
}


}
