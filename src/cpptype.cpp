/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Cedric GESTES
*/
#include <sstream>
#include "cpptype.hpp"
#include "formatter_p.hpp"
#include <boost/algorithm/string.hpp>
#include <qilang/visitor.hpp>
#include <qi/qi.hpp>
#include <qi/path.hpp>

qiLogCategory("qilang.cpp");


namespace qilang {

static std::string constRefYourSelf(const std::string& type, bool constref) {
  if (!constref)
    return type;
  return "const " + type + "&";
}

static std::string builtinTypeToCpp(BuiltinType type, bool constref) {
  switch (type) {
    case BuiltinType_Nothing:
      return "void";
    case BuiltinType_Bool:
      return "bool";
    case BuiltinType_Char:
      return "char";
    case BuiltinType_Int:
      return "int";
    case BuiltinType_UInt:
      return "unsigned int";
    case BuiltinType_Int8:
      return "qi::int8_t";
    case BuiltinType_UInt8:
      return "qi::uint8_t";
    case BuiltinType_Int16:
      return "qi::int16_t";
    case BuiltinType_UInt16:
      return "qi::uint16_t";
    case BuiltinType_Int32:
      return "qi::int32_t";
    case BuiltinType_UInt32:
      return "qi::uint32_t";
    case BuiltinType_Int64:
      return "qi::int64_t";
    case BuiltinType_UInt64:
      return "qi::uint64_t";
    case BuiltinType_Float:
      return "float";
    case BuiltinType_Float32:
      return "float";
    case BuiltinType_Float64:
      return "double";
    case BuiltinType_String:
      return constRefYourSelf("std::string", constref);
    case BuiltinType_Value:
      return constRefYourSelf("qi::AnyValue", constref);
    case BuiltinType_Object:
      return constRefYourSelf("qi::AnyObject", constref);
  }
  throw std::runtime_error("unreachable code");
}


static std::string toName(const std::string& name, int counter) {
  if (name != "_")
   return name;
  std::stringstream ss;
  ss << "arg";
  ss << counter;
  return ss.str();
}

static void cppFormatParam(CppTypeFormatter* fmt, ParamFieldDeclNodePtr node, CppParamsFormat cfpt, int counter) {
  for (unsigned i = 0; i < node->names.size(); ++i) {
    switch(node->paramType) {
      case ParamFieldType_Normal: {
        if (cfpt != CppParamsFormat_NameOnly) {
          fmt->constify(node->effectiveType());
          fmt->out() << " ";
        }
        if (cfpt != CppParamsFormat_TypeOnly)
          fmt->out() << toName(node->names.at(i), counter);
        break;
      }
      case ParamFieldType_VarArgs: {
        if (cfpt != CppParamsFormat_NameOnly) {
          ScopedFormatAttrActivate _(fmt->constattr);
          fmt->accept(node->effectiveType());
        }
        if (cfpt != CppParamsFormat_TypeOnly)
          fmt->out() << toName(node->names.at(i), counter);
        break;
      }
      case ParamFieldType_KeywordArgs: {
        if (cfpt != CppParamsFormat_NameOnly) {
          ScopedFormatAttrActivate _(fmt->constattr);
          fmt->accept(node->effectiveType());
        }
        if (cfpt != CppParamsFormat_TypeOnly)
          fmt->out() << toName(node->names.at(i), counter);
        break;
      }
    }
  }
}

void cppParamsFormat(CppTypeFormatter* typeformat, ParamFieldDeclNodePtrVector params, CppParamsFormat cfpt) {
  for (unsigned i = 0; i < params.size(); ++i) {
    cppFormatParam(typeformat, params.at(i), cfpt, i);
    if (i + 1 < params.size())
      typeformat->out() << ", ";
  }
}



CppTypeFormatter::CppTypeFormatter()
{
}

void CppTypeFormatter::unconstify(TypeExprNodePtr node) {
  ScopedFormatAttrBlock _(constattr);
  accept(node);
}

void CppTypeFormatter::constify(const TypeExprNodePtr& node) {
  ScopedFormatAttrActivate _(constattr);
  accept(node);
}

void CppTypeFormatter::visitTypeExpr(BuiltinTypeExprNode* node) {
  out() << builtinTypeToCpp(node->builtinType, constattr.isActive());
}
void CppTypeFormatter::visitTypeExpr(CustomTypeExprNode* node) {

  std::string ns = formatNs(node->resolved_package);

  out() << constattr("const ");
  if (!ns.empty())
    out() << ns << "::" << node->resolved_value << constattr("&");
}
void CppTypeFormatter::visitTypeExpr(ListTypeExprNode* node) {
  out() << constattr("const ") << "std::vector< ";
  unconstify(node->element);
  out() << " >" << constattr("&");
}
void CppTypeFormatter::visitTypeExpr(MapTypeExprNode* node) {
  out() << constattr("const ") << "std::map< ";
  unconstify(node->key);
  out() << ", ";
  unconstify(node->value);
  out() << " >" << constattr("&");
}
void CppTypeFormatter::visitTypeExpr(TupleTypeExprNode* node) {
  if (node->elements.size() == 2) {
    out() << constattr("const ") << "std::pair< ";
    unconstify(node->elements.at(0));
    out() << ", ";
    unconstify(node->elements.at(1));
    out() << " >" << constattr("&");
  }
  else
    out() << "TUPLENOTIMPL";
}

void CppTypeFormatter::visitTypeExpr(VarArgTypeExprNode* node) {
  out() << constattr("const ") << "qi::VarArguments< ";
  unconstify(node->element);
  out() << " >" << constattr("&");
}

void CppTypeFormatter::visitTypeExpr(KeywordArgTypeExprNode* node) {
  out() << constattr("const ") << "qi::KeywordArguments< ";
  unconstify(node->value);
  out() << " >" << constattr("&");
}

void CppTypeFormatter::visitData(BoolLiteralNode *node) {
  if (node->value)
    out() << "true";
  else
    out() << "false";
}
void CppTypeFormatter::visitData(IntLiteralNode *node) {
  out() << node->value;
}
void CppTypeFormatter::visitData(FloatLiteralNode *node) {
  out() << node->value;
}
void CppTypeFormatter::visitData(StringLiteralNode *node) {
  out() << node->value;
}
void CppTypeFormatter::visitData(TupleLiteralNode* node) {
  out() << "(" << "FAIL" << ")";
}
void CppTypeFormatter::visitData(ListLiteralNode* node) {
  out() << "[" << "FAIL" << "]";
}
void CppTypeFormatter::visitData(DictLiteralNode* node) {
  out() << "{" << "FAIL" << "}";
}

void CppTypeFormatter::visitExpr(BinaryOpExprNode *node) {
  throw std::runtime_error("unimplemented");
}
void CppTypeFormatter::visitExpr(UnaryOpExprNode *node) {
  throw std::runtime_error("unimplemented");
}
void CppTypeFormatter::visitExpr(VarExprNode *node) {
  //throw std::runtime_error("unimplemented");
}
void CppTypeFormatter::visitExpr(LiteralExprNode* node) {
  throw std::runtime_error("unimplemented");
}

void CppTypeFormatter::visitExpr(CallExprNode* node) {
  out() << node->name << "(";
  for (unsigned i = 0; i < node->args.size(); ++i) {
    accept(node->args.at(i));
    if (i + 1 != node->args.size())
      out() << ", ";
  }
  out() << ")";
}

static std::string stripQiLangExtension(const std::string& name)
{
  if (boost::ends_with(name, ".idl.qi"))
    return name.substr(0, name.size() - 7);
  if (boost::ends_with(name, ".qi"))
    return name.substr(0, name.size() - 3);
  return name;
}

std::vector<std::string> splitPkgName(const std::string& name) {
  std::vector<std::string> ret;

  boost::split(ret, name, boost::is_any_of("."));
  return ret;
}

void formatBlock(std::ostream& os, const std::string& name, const std::string& sep, int indent) {
  StringVector lines;

  lines = boost::split(lines, name, boost::is_any_of("\r\n"));
  for (unsigned int i = 0; i < lines.size(); ++i) {
    std::string& line = lines.at(i);
    for (int j = 0; j < indent; ++j)
      os << " ";
    os << sep << line << std::endl;
  }
}

std::string pkgNameToDir(const std::string& name) {
  std::string ret(name);
  boost::replace_all(ret, ".", "/");
  return ret;
}

std::string pkgNameToAPI(const std::string& name) {
  std::string ret(name);
  boost::replace_all(ret, ".", "_");
  boost::to_upper(ret);
  ret += "_API";
  return ret;
}

std::string filenameToCppHeaderGuard(const std::string &pkgName, const std::string &filename)
{
  qi::Path p(filename);
  p = p.filename();

  std::string ret(pkgName);
  boost::replace_all(ret, ".", "_");
  ret += "_" + stripQiLangExtension(p);
  boost::to_upper(ret);
  ret = "_QILANG_GEN_" + ret + "_";
  return ret;
}


std::string formatNs(const std::string& package) {
  std::string ret;
  std::vector<std::string> v;

  boost::split(v, package, boost::is_any_of("."));

  if (v.size() > 0)
    ret += "::";
  for (unsigned int i = 0; i < v.size(); ++i) {
    ret += v.at(i);
    if (i + 1 < v.size())
      ret += "::";
  }
  return ret;
}

template <typename T, typename U>
void pushIfNot(std::vector<T>& v, const U& elt) {
  if (std::find(v.begin(), v.end(), elt) == v.end())
    v.push_back(elt);
}

static StringVector filenameFromImport(const PackagePtr& pkg, ImportNode* tnode) {
  StringVector ret;
  //for each symbol in importnode
  if (tnode->importType == ImportType_Package || tnode->importType == ImportType_All) {
    return pkg->files();
  }
  for (unsigned i = 0; i < tnode->imports.size(); ++i) {
    std::string& name = tnode->imports.at(i);
    pushIfNot(ret, pkg->fileFromExport(name));
  }
  return ret;
}


static std::string qiLangToCppInclude(const PackagePtr& pkg, const std::string& filename) {
  qi::Path pkgpath(pkgNameToDir(pkg->_name));
  qi::Path fpath(stripQiLangExtension(filename));
  return "<" + (std::string)(pkgpath / fpath.filename()) + ".hpp>";
}

static StringVector cppFilenameFromImport(const PackagePtr& pkg, ImportNode* tnode) {
  StringVector ret;
  StringVector fnames = filenameFromImport(pkg, tnode);

  for (unsigned j = 0; j < fnames.size(); ++j) {
    std::string& name = fnames.at(j);
    ret.push_back(qiLangToCppInclude(pkg, name));
  }
  return ret;
}

StringVector extractCppIncludeDir(const PackageManagerPtr& pm, const ParseResultPtr& pr, bool self) {
  StringVector  includes;
  NodePtrVector imports;
  NodePtrVector typeExprs;
  NodePtrVector decls;

  pushIfNot(includes, qiLangToCppInclude(pm->package(pr->package), "api"));
  if (self) {
    pushIfNot(includes, qiLangToCppInclude(pm->package(pr->package), pr->filename) + " //self");
  }
  //for each import generate the include.
  imports = findNode(pr->ast, NodeType_Import);
  for (unsigned i = 0; i < imports.size(); ++i) {
    ImportNode* tnode = static_cast<ImportNode*>(imports.at(i).get());
    PackagePtr pkg = pm->package(tnode->name);
    StringVector sv = cppFilenameFromImport(pkg, tnode);
    for (unsigned j = 0; j < sv.size(); ++j) {
      pushIfNot(includes, sv.at(i));
    }
  }

  //for each TypeExpr generate include as appropriated  (for builtin types)
  typeExprs = findNode(pr->ast, NodeKind_TypeExpr);
  for (unsigned i = 0; i < typeExprs.size(); ++i) {
    NodePtr& node = typeExprs.at(i);
    switch (node->type()) {
      case NodeType_TupleTypeExpr: {
        TupleTypeExprNode* tnode = static_cast<TupleTypeExprNode*>(node.get());
        if (tnode->elements.size() == 2)
          pushIfNot(includes, "<pair>");
        else {
          pushIfNot(includes, "NOTIMPLTUPLE");
          qiLogWarning() << "BUG: include handling for tuple with size != 2 not handled";
        }
        break;
      }
      case NodeType_ListTypeExpr:
        pushIfNot(includes, "<vector>");
        break;
      case NodeType_MapTypeExpr:
        pushIfNot(includes, "<map>");
        break;
      case NodeType_BuiltinTypeExpr: {
        BuiltinTypeExprNode* tnode = static_cast<BuiltinTypeExprNode*>(node.get());
        if (tnode->value == "str") {
          pushIfNot(includes, "<string>");
        } else if (tnode->value == "any") {
          pushIfNot(includes, "<qi/anyvalue.hpp>");
        } else if (tnode->value == "obj") {
          pushIfNot(includes, "<qi/anyobject.hpp>");
        } else {
          pushIfNot(includes, "<qi/types.hpp>");
        }
        break;
      }
      case NodeType_KeywordArgTypeExpr:
      case NodeType_VarArgTypeExpr: {
        pushIfNot(includes, "<qi/anyfunction.hpp>");
        break;
      }
      case NodeType_CustomTypeExpr: {
        //break;
        //already handled by imports
        CustomTypeExprNode* tnode = static_cast<CustomTypeExprNode*>(node.get());
        PackagePtr pkg = pm->package(tnode->resolved_package);
        pushIfNot(includes, qiLangToCppInclude(pkg, pkg->fileFromExport(tnode->resolved_value)));
        break;
      }
      default:
        break;
    }
  }

  //for each TypeExpr generate include as appropriated  (for builtin types)
  decls = findNode(pr->ast, NodeKind_Decl);
  for (unsigned i = 0; i < decls.size(); ++i) {
    NodePtr& node = decls.at(i);
    switch (node->type()) {
      case NodeType_EmitDecl:
        pushIfNot(includes, "<qi/signal.hpp>");
        break;
      case NodeType_PropDecl:
        pushIfNot(includes, "<qi/property.hpp>");
        break;
      case NodeType_InterfaceDecl:
        pushIfNot(includes, "<qi/anyobject.hpp>");
        break;
      default:
        break;
    }
  }
  return includes;
}


}
