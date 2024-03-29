/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Cedric GESTES
*/
#include <sstream>
#include "formatter_p.hpp"
#include <boost/algorithm/string.hpp>
#include <qilang/visitor.hpp>
#include <qi/path.hpp>
#include <qilang/packagemanager.hpp>

qiLogCategory("qilang.cpp");

namespace qilang {

static std::string constRefYourSelf(const std::string& type, bool constref) {
  if (!constref)
    return type;
  return "const " + type + "&";
}

namespace detail {

std::string builtinTypeToCpp(BuiltinType type, bool constref) {
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
    case BuiltinType_NanoSeconds:
      return "qi::NanoSeconds";
    case BuiltinType_MicroSeconds:
      return "qi::MicroSeconds";
    case BuiltinType_MilliSeconds:
      return "qi::MilliSeconds";
    case BuiltinType_Seconds:
      return "qi::Seconds";
    case BuiltinType_Minutes:
      return "qi::Minutes";
    case BuiltinType_Hours:
      return "qi::Hours";
    case BuiltinType_QiTimePoint:
      return "qi::ClockTimePoint";
    case BuiltinType_SteadyTimePoint:
      return "qi::SteadyClockTimePoint";
    case BuiltinType_SystemTimePoint:
      return "qi::SystemClockTimePoint";
    case BuiltinType_String:
      return constRefYourSelf("std::string", constref);
    case BuiltinType_Raw:
      return constRefYourSelf("qi::Buffer", constref);
    case BuiltinType_Value:
      return constRefYourSelf("qi::AnyValue", constref);
    case BuiltinType_Object:
      return constRefYourSelf("qi::AnyObject", constref);
  }
  throw std::runtime_error("unreachable code");
}

std::string toName(const std::string& name, int counter) {
  if (name != "_")
   return name;
  std::stringstream ss;
  ss << "arg";
  ss << counter;
  return ss.str();
}

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

std::string filenameToHeaderGuardBase(
    const std::string& pkgName,
    const std::string& filename)
{
  qi::Path p(filename);
  p = p.filename();

  std::string ret(pkgName);
  boost::replace_all(ret, ".", "_");
  ret += "_" + stripQiLangExtension(p.str());
  boost::to_upper(ret);
  return ret;
}

std::string filenameToInterfaceHeaderGuard(const std::string &pkgName, const std::string &filename)
{
  return "_QILANG_GEN_INTERFACE_" + filenameToHeaderGuardBase(pkgName, filename) + "_HPP";
}

std::string filenameToLocalHeaderGuard(const std::string &pkgName, const std::string &filename)
{
  return "_QILANG_GEN_LOCAL_" + filenameToHeaderGuardBase(pkgName, filename) + "_P_HPP";
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


std::string qiLangToCppInclude(const PackagePtr& pkg, const std::string& filename) {
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
      pushIfNot(includes, sv.at(j));
    }
  }

  //for each TypeExpr generate include as appropriate (for built-in types)
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
        } else if (tnode->value == "raw") {
          pushIfNot(includes, "<qi/buffer.hpp>");
        } else if (tnode->value == "any") {
          pushIfNot(includes, "<qi/anyvalue.hpp>");
        } else if (tnode->value == "obj") {
          pushIfNot(includes, "<qi/anyobject.hpp>");
        } else if (tnode->value == "nsec"
                || tnode->value == "usec"
                || tnode->value == "msec"
                || tnode->value == "sec"
                || tnode->value == "min"
                || tnode->value == "hour"
                || tnode->value == "qitimepoint"
                || tnode->value == "steadytimepoint"
                || tnode->value == "systemtimepoint") {
          pushIfNot(includes, "<qi/clock.hpp>");
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
      case NodeType_SigDecl:
        pushIfNot(includes, "<qi/signal.hpp>");
        pushIfNot(includes, "<qi/type/proxysignal.hpp>");
        break;
      case NodeType_PropDecl:
        pushIfNot(includes, "<qi/property.hpp>");
        pushIfNot(includes, "<qi/type/proxyproperty.hpp>");
        break;
      case NodeType_StructDecl:
      case NodeType_InterfaceDecl:
        pushIfNot(includes, "<qi/anyobject.hpp>");
        break;
      case NodeType_EnumDecl:
        pushIfNot(includes, "<qi/type/typeinterface.hpp>");
        break;
      default:
        break;
    }
  }
  return includes;
}


}
