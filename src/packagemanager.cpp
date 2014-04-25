/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <iostream>
#include <qilang/packagemanager.hpp>
#include <qilang/parser.hpp>
#include <qilang/visitor.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem.hpp>
#include <qi/qi.hpp>

namespace fs = boost::filesystem;

namespace qilang {

  void packageVisitor(const NodePtr& parent, const NodePtr& node, qilang::StringVector& result) {
    switch(node->type()) {
      case NodeType_Package: {
        PackageNode* tnode = dynamic_cast<PackageNode*>(node.get());
        result.push_back(tnode->name);
        break;
      } default:
        break;
    };
  }

  void importExportDeclVisitor(const NodePtr& parent, const NodePtr& node, PackagePtr& pkg) {
    //we care only about toplevel decl
    if (parent)
      return;

    switch (node->type()) {
      case NodeType_InterfaceDecl: {
        InterfaceDeclNode* tnode = dynamic_cast<InterfaceDeclNode*>(node.get());
        pkg->addMember(tnode->name, node);
        return;
      } case NodeType_StructDecl: {
        StructDeclNode* tnode = dynamic_cast<StructDeclNode*>(node.get());
        pkg->addMember(tnode->name, node);
        return;
      } case NodeType_FnDecl: {
        FnDeclNode* tnode = dynamic_cast<FnDeclNode*>(node.get());
        pkg->addMember(tnode->name, node);
        return;
      } case NodeType_ConstDecl: {
        ConstDeclNode* tnode = dynamic_cast<ConstDeclNode*>(node.get());
        pkg->addMember(tnode->name, node);
        return;
      } case NodeType_ObjectDef: {
        ObjectDefNode* tnode = dynamic_cast<ObjectDefNode*>(node.get());
        pkg->addMember(tnode->name, node);
        return;
      } case NodeType_Import: {
        ImportNode* tnode = dynamic_cast<ImportNode*>(node.get());
        pkg->addImport(tnode->name, node);
        return;
      }
      default: {
        return;
      }
    }
  }

  PackagePtr    PackageManager::package(const std::string& packagename) {
    PackagePtrMap::iterator it;
    it = _packages.find(packagename);
    if (it == _packages.end())
      throw std::runtime_error("package not found: " + packagename);
    return it->second;
  }

  //check the path of a file is correct. (package match)
  //return the fullpath of the package
  static std::string checkPackagePath(const std::string& filename, const std::string& package)
  {
    fs::path p(filename, qi::unicodeFacet());

    std::string par = p.parent_path().filename().string(qi::unicodeFacet());
    if (par != package)
      throw std::runtime_error("Error: '" + filename + "' define package '" + package +
                               "' but the parent folder is not correct: '" + par + "'. Package and directory should match");
    return fs::absolute(p.parent_path().parent_path()).string(qi::unicodeFacet());
  }

  NodePtrVector PackageManager::parseFile(const std::string& fname) {
    std::string filename = fs::absolute(fs::path(fname, qi::unicodeFacet())).string(qi::unicodeFacet());
    qiLogVerbose() << "Parsing file: " << filename;

    if (_sources.find(filename) != _sources.end()) {
      qiLogVerbose() << "already parsed, skipping '" << filename << "'";
      return _sources[filename];
    }
    NodePtrVector ret = qilang::parse(filename);
    _sources[filename] = ret;

    StringVector sv;
    visitNode(ret, boost::bind<void>(&packageVisitor, _1, _2, boost::ref(sv)));

    //TODO: handle error location...
    if (sv.size() != 1)
      throw std::runtime_error("0 or >1 package definition");

    std::string pkgname = sv[0];

    std::string path = checkPackagePath(filename, pkgname);
    addInclude(path);

    addPackage(pkgname);
    package(pkgname)->setContent(filename, ret);
    return ret;
  }


  static bool locateFileInPackage(const std::string& path, const std::string& package, StringVector* result) {
    fs::directory_iterator dit(fs::path(path, qi::unicodeFacet()));
    bool ret = false;
    for (; dit != fs::directory_iterator(); ++dit) {
      fs::path p = *dit;

      if (fs::is_directory(p)) {
        StringVector sv;
        bool b = locateFileInPackage(p.string(qi::unicodeFacet()), package, &sv);
        if (b) {
          result->insert(result->end(), sv.begin(), sv.end());
          ret = true;
        }
      }
      if (fs::is_regular_file(p)) {
        if (p.extension() == ".qi") {
          qiLogVerbose() << "locate file in '" << package << "' found:" << p.string(qi::unicodeFacet());
          result->push_back(p.string(qi::unicodeFacet()));
          ret = true;
        }
      }
    }
    return ret;
  }

  void         PackageManager::addInclude(const std::string& include) {
    if (std::find(_includes.begin(), _includes.end(), include) == _includes.end())
      _includes.insert(_includes.begin(), include);
  }

  StringVector PackageManager::locatePackage(const std::string& pkgName) {
    StringVector ret;

    for (unsigned i = 0; i < _includes.size(); ++i) {
      fs::path p(_includes.at(i), qi::unicodeFacet());
      fs::directory_iterator dit(p);

      for (; dit != fs::directory_iterator(); ++dit) {
        fs::path pd = *dit;
        if (fs::is_directory(pd) && pd.filename().string(qi::unicodeFacet()) == pkgName) {
          bool b = locateFileInPackage(fs::absolute(pd).string(qi::unicodeFacet()), pkgName, &ret);
          if (b) {
            qiLogVerbose() << "Found pkg '" << pkgName << "'";
            return ret;
          }
        }
      }
    }
    return ret;
  }


  /**
   * @brief PackageManager::parsePackage
   * @param package
   *
   * locate the package... use QIPATH and -I to look for the package.
   * a folder with a "pkgname".pkg.qi file
   */
  void PackageManager::parsePackage(const std::string& packageName) {
    StringVector sv = locatePackage(packageName);

    for (unsigned i = 0; i < sv.size(); ++i) {
      parseFile(sv.at(i));
    }

    // for each decl in the package. reference it into the package.
    PackagePtr pkg = package(packageName);


    ASTMap::iterator it;
    for (it = pkg->_contents.begin(); it != pkg->_contents.end(); ++it) {
      qiLogVerbose() << "Visiting: " << it->first;
      visitNode(it->second, boost::bind<void>(&importExportDeclVisitor, _1, _2, boost::ref(pkg)));
    }
    pkg->dump();

  }

  /**
   * @brief PackageManager::anal
   * @param packageName
   *
   * let's annalyse what we have..
   * precond: we have a list of files already parsed in the asked package.
   *
   * first pass: we find all exported symbols of a package
   * second pass: we resolve all TypeExpr
   */
  void PackageManager::anal(const std::string &packageName) {
    qiLogVerbose() << "SemAnal pkg:" << packageName;

    parsePackage(packageName);
  }

};
