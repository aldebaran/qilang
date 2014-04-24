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

    void addMember(const std::string& member, const NodePtr& node) {
      NodeMap::iterator it;
      for (it = _refs.begin(); it != _refs.end(); ++it) {
        if (it->first == member)
          throw std::runtime_error("symbol already exported:" + qilang::format(it->second));
      }
      qiLogVerbose() << "Added symbol '" << member << "' to package " << _name;
      //ok add the symbol
      _refs[member] = node;
    }

    void setContent(const std::string& filename, const NodePtrVector& nodes) {
      if (_contents.find(filename) != _contents.end())
        throw std::runtime_error("content already set for file: " + filename);
      _contents[filename] = nodes;
    }

    std::string _name;
    NodeMap     _refs;      //list of exported symbol
    ASTMap      _contents;  //
  };

  typedef boost::shared_ptr<Package>        PackagePtr;
  typedef std::map<std::string, PackagePtr> PackagePtrMap;


  /**
   *
   *
   */
  class QILANG_API PackageManager {
  public:
    PackageManager() {}

    NodePtrVector parseFile(const std::string& filename);
    void parsePackage(const std::string& package);

    PackagePtr addPackage(const std::string& name) {
      if (_packages.find(name) != _packages.end())
        return _packages[name];
      _packages[name] = boost::make_shared<Package>(name);
      return _packages[name];
    }

    void anal(const std::string& package);

    NodePtrVector ast(const std::string& filename);
    PackagePtr    package(const std::string& packagename);

  protected:
    PackagePtrMap _packages;
    ASTMap        _sources;
    StringVector  _packagespath;

  };

}

#endif	    /* !PACKAGEMANAGER_PP_ */
