/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#ifndef   	PACKAGEMANAGER_HPP_
# define   	PACKAGEMANAGER_HPP_

#include <unordered_set>
#include <qilang/api.hpp>
#include <qilang/node.hpp>
#include <qilang/parser.hpp>
#include <qilang/formatter.hpp>
#include <map>
#include <string>
#include <vector>
#include <boost/make_shared.hpp>


namespace qilang {

  typedef std::map<std::string, std::string>   FilenameToPackageMap;
  typedef std::map<std::string, ParseResultPtr>   ParseResultMap;
  typedef std::vector<ParseResultPtr>             ParseResultVector;
  typedef std::map<std::string, NodePtrVector> ASTMap;
  typedef std::map<std::string, NodePtr>       NodeMap;

  /** Describe a package
   *
   * _contents contains all files of the Package
   * _exported contains the name of all exported symbol
   */
  class QILANG_API Package {
  public:

    Package(const std::string& name)
      : _name(name)
      , _parsed(false)
    {}

    void addImport(const std::string& import, const NodePtr& node) {
      qiLogCategory("qilang.pm");
      qiLogVerbose() << "Added import '" << import << "' to package " << _name;
      //ok add the symbol
      _imports[import].push_back(node);
    }

    void addMember(const std::string& member, const NodePtr& node) {
      qiLogCategory("qilang.pm");
      NodeMap::iterator it;
      for (it = _exports.begin(); it != _exports.end(); ++it) {
        if (it->first == member)
          throw std::runtime_error("symbol " + _name + "." + member +
                                   "\ndefined by\n" +
                                   node->loc().filename +
                                   "\nis already defined by\n" +
                                   it->second->loc().filename);
      }
      qiLogVerbose() << "Added export '" << member << "' to package " << _name;
      //ok add the symbol
      _exports[member] = node;
    }

    NodePtr getExport(const std::string& decl) {
       NodeMap::const_iterator it;
       qiLogCategory("qilang.pm");
       qiLogVerbose() << _name << " looking for export: " << decl;
       it = _exports.find(decl);
       if (it == _exports.end())
         return NodePtr();
       qiLogVerbose() << _name << " found export: " << decl;
       return it->second;
    }

    void setContent(const std::string& filename, const ParseResultPtr& result) {
      if (_contents.find(filename) != _contents.end())
        throw std::runtime_error("content already set for file: " + filename);
      _contents[filename] = result;
    }

    void dump() {
      NodeMap::iterator it;
      for (it = _exports.begin(); it != _exports.end(); ++it) {
        std::cout << "refs:" << _name << "." << it->first << std::endl;
      }
    }

    StringVector files() {
      ParseResultMap::const_iterator it;
      StringVector ret;
      for (it = _contents.begin(); it != _contents.end(); ++it) {
        ret.push_back(it->first);
      }
      return ret;
    }

    std::string fileFromExport(const std::string& name) {
      NodeMap::const_iterator it = _exports.find(name);

      if (it == _exports.end())
        throw std::runtime_error("export symbol '" + name + "' not found in package '" + this->_name + "'");
      return _exports.at(name)->loc().filename;
    }

    bool hasError() const;
    void printMessage(std::ostream& os) const;

    std::string    _name;      // package name
    ParseResultMap _contents;  // map<filename, ParseResult>  file of the package
    NodeMap        _exports;   // map<membername, Node> package exported symbol
    ASTMap         _imports;   // map<pkgname, Nodes>   list of declared imports
    bool           _parsed;    // true if each files of the package are parsed
  };

  typedef boost::shared_ptr<Package>        PackagePtr;
  typedef std::map<std::string, PackagePtr> PackagePtrMap;

  //typedef std::map<std::string, DiagnosticVector> DiagnosticMap;

  class DiagnosticManager {
  public:
    void add(const Diagnostic& msg);

    DiagnosticVector _diags;
  };

  typedef boost::shared_ptr<DiagnosticManager> DiagnosticManagerPtr;

  struct ResolutionResult {
    std::string pkg;
    std::string type;
    TypeKind kind;

    ResolutionResult() {}
    ResolutionResult(const std::string& pkg, const std::string& type, TypeKind kind)
      : pkg(pkg)
      , type(type)
      , kind(kind)
    {}
  };

  /**
   *  PackageManager...
   *
   *  goals? gain knowledge about package and vars. (semantic pass)
   *
   *  we have files to compile...
   *
   *  we need to find the package they belong. we need to parse all files of the package.
   *  we need to parse all dependents packages.
   *
   */
  class QILANG_API PackageManager {
  public:
    PackageManager()
      //: _diag(new DiagnosticManager)
    {}

    ParseResultPtr parseFile(const FileReaderPtr& file);
    void parsePackage(const std::string& packageName);

    void addLookupPaths(const StringVector& lookupPaths);
    void anal(const std::string& package = std::string());

    NodePtrVector ast(const std::string& filename);
    PackagePtr    package(const std::string& packagename) const;

    void         addInclude(const std::string& include);
    void         setIncludes(const StringVector& includes) { _includes = includes; }
    StringVector includes() const                          { return _includes; }

    //return all the files composing a package  (their may be false)
    std::unordered_set<std::string> locatePackage(const std::string& pkgName);

    bool hasError() const;
    void printMessage(std::ostream& os) const;

    void parseDir(const std::string &dirname);

    ResolutionResult resolveImport(const ParseResultPtr& pr, const PackagePtr &pkg, const CustomTypeExprNode* node);

  protected:
    PackagePtr addPackage(const std::string& name) {
      if (_packages.find(name) != _packages.end())
        return _packages[name];
      _packages[name] = boost::make_shared<Package>(name);
      return _packages[name];
    }
    bool addFileToPackage(const std::string& absfile, const FileReaderPtr& file, ParseResultPtr& ret);
    void resolvePackage(const std::string &packageName);

  protected:
    PackagePtrMap        _packages; // packagename , packageptr
    FilenameToPackageMap _sources;  // abs filename , packagename
    StringVector         _includes;
    StringVector _lookupPaths;
  };
  typedef boost::shared_ptr<PackageManager> PackageManagerPtr;
  inline PackageManagerPtr newPackageManager() { return boost::make_shared<PackageManager>(); }

}

#endif	    /* !PACKAGEMANAGER_PP_ */
