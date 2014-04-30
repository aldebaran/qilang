/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#ifndef   	PACKAGEMANAGER_HPP_
# define   	PACKAGEMANAGER_HPP_

#include <qilang/api.hpp>
#include <qilang/node.hpp>
#include <qilang/formatter.hpp>
#include <map>
#include <boost/make_shared.hpp>

qiLogCategory("qilang.pm");

namespace qilang {

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
    {}

    void addImport(const std::string& import, const NodePtr& node) {
      qiLogVerbose() << "Added import '" << import << "' to package " << _name;
      //ok add the symbol
      _imports[import].push_back(node);
    }

    void addMember(const std::string& member, const NodePtr& node) {
      NodeMap::iterator it;
      for (it = _exports.begin(); it != _exports.end(); ++it) {
        if (it->first == member)
          throw std::runtime_error("symbol already exported:" + qilang::format(it->second));
      }
      qiLogVerbose() << "Added export '" << member << "' to package " << _name;
      //ok add the symbol
      _exports[member] = node;
    }

    void setContent(const std::string& filename, const NodePtrVector& nodes) {
      if (_contents.find(filename) != _contents.end())
        throw std::runtime_error("content already set for file: " + filename);
      _contents[filename] = nodes;
    }

    void dump() {
      NodeMap::iterator it;
      for (it = _exports.begin(); it != _exports.end(); ++it) {
        std::cout << "refs:" << _name << "." << it->first << std::endl;
      }
    }

    std::string  _name;      // package name
    NodeMap      _exports;   // map<membername, Node> package exported symbol
    ASTMap       _contents;  // map<filename, Nodes>  file of the package
    ASTMap       _imports;   // map<pkgname, Nodes>   list of depends packages
  };

  typedef boost::shared_ptr<Package>        PackagePtr;
  typedef std::map<std::string, PackagePtr> PackagePtrMap;


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
    PackageManager() {}

    NodePtrVector parseFile(const std::string& filename);
    void parsePackage(const std::string& packageName);

    PackagePtr addPackage(const std::string& name) {
      if (_packages.find(name) != _packages.end())
        return _packages[name];
      _packages[name] = boost::make_shared<Package>(name);
      return _packages[name];
    }

    void anal(const std::string& package = std::string());

    NodePtrVector ast(const std::string& filename);
    PackagePtr    package(const std::string& packagename);

    void         addInclude(const std::string& include);
    void         setIncludes(const StringVector& includes) { _includes = includes; }
    StringVector includes() const                          { return _includes; }

    //return all the files composing a package  (their may be false)
    StringVector locatePackage(const std::string& pkgName);
  protected:
    PackagePtrMap _packages;
    ASTMap        _sources;
    //StringVector  _packagespath;
    StringVector  _includes;
  };

}

#endif	    /* !PACKAGEMANAGER_PP_ */
