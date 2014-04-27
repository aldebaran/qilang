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
#include "cpptype.hpp"
#include <boost/make_shared.hpp>
#include <qi/qi.hpp>
#include <qi/path.hpp>
qiLogCategory("qilang.pm");

namespace qilang {

  std::string extractPackageName(const NodePtr& node) {
    switch(node->type()) {
      case NodeType_Package: {
        PackageNode* tnode = dynamic_cast<PackageNode*>(node.get());
        return tnode->name;
      } default:
        throw std::runtime_error("node is not a package");
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
      throw std::runtime_error("package not found '" + packagename + "'");
    return it->second;
  }

  /** 1 / Check for missing or multiple package declaration
   *  2 / Check that the directory path and package name match
   *  3 / register the content of the file to the package
   */
  bool PackageManager::addFileToPackage(const std::string& absfile, const FileReaderPtr& file, ParseResult& ret) {

    // 1
    NodePtrVector result;
    result = findNode(ret.ast, NodeType_Package);
    if (result.size() == 0) {
      ret.messages.push_back(Message(MessageType_Error, "missing package declaration", Location(file->filename())));
      return false;
    }
    if (result.size() > 1) {
      for (unsigned i = 1; i < result.size(); ++i) {
        ret.messages.push_back(Message(MessageType_Error, "extra package declaration", result.at(i)->loc()));

      }
      ret.messages.push_back(Message(MessageType_Info, "previous declared here", result.at(0)->loc()));
      return false;
    }
    std::string pkgname = extractPackageName(result.at(0));
    // 2
    qi::Path pf(file->filename());
    StringVector leafs = splitPkgName(pkgname);


    qi::Path dirname;
    qi::Path cur = pf.parent().absolute();
    for (unsigned i = 0; i < leafs.size(); ++i)
    {
      dirname = qi::Path(cur.filename()) / dirname;
      cur = cur.parent();
      if (cur.isEmpty())
        break;
    }

    qi::Path p = pf;
    for (int i = leafs.size() - 1; i >= 0; --i) {
      std::string par = p.parent().filename();
      if (par != leafs.at(i)) {
        ret.messages.push_back(Message(MessageType_Error,
                                       "package name '" + pkgname + "' do not match parent directory name '" + (std::string)dirname + "'",
                                       result.at(0)->loc()));
        return false;
      }
      p = p.parent();
    }

    std::string pkgpath = p.parent().absolute();

    addInclude(pkgpath);
    addPackage(pkgname);
    ret.package = pkgname;
    package(pkgname)->setContent(absfile, ret);
    return true;
  }

  ParseResult PackageManager::parseFile(const FileReaderPtr& file)
  {
    ParseResult ret = _parseFile(file);
    if (ret.hasError())
      return ret;
    parsePackage(ret.package);
    return ret;
  }

  ParseResult PackageManager::_parseFile(const FileReaderPtr& file)
  {
    qi::Path fsfname = qi::Path(file->filename());
    std::string filename = fsfname.absolute();
    if (!fsfname.isRegularFile())
      throw std::runtime_error(file->filename() + " is not a regular file");
    qiLogVerbose() << "Parsing file: " << filename;
    if (_sources.find(filename) != _sources.end()) {
      qiLogVerbose() << "already parsed, skipping '" << filename << "'";
      return package(_sources[filename])->_contents[filename];
    }

    ParseResult ret = qilang::parse(file);
    if (addFileToPackage(filename, file, ret))
      _sources[filename] = ret.package;
    return ret;
  }


  static bool locateFileInDir(const std::string& path, StringVector* resultfile, StringVector* resultdir) {
    qi::PathVector pv = qi::Path(path).dirs();
    bool ret = false;
    for (unsigned i = 0; i < pv.size(); ++i) {
      qi::Path& p = pv.at(i);
      resultdir->push_back(p);
    }

    pv = qi::Path(path).files();
    for (unsigned i = 0; i < pv.size(); ++i) {
      qi::Path& p = pv.at(i);
      if (p.isRegularFile()) {
        if (p.extension() == ".qi") {
          resultfile->push_back(p);
          ret = true;
        }
      }
    }
    return ret;
  }

  void         PackageManager::addInclude(const std::string& include) {
    if (std::find(_includes.begin(), _includes.end(), include) == _includes.end()) {
      qiLogVerbose() << "adding include: " << include;
      _includes.insert(_includes.begin(), include);
    }
  }

  StringVector PackageManager::locatePackage(const std::string& pkgName) {

    if (pkgName.empty())
      throw std::runtime_error("empty package name");

    qi::Path pkgPath(pkgNameToDir(pkgName));

    for (unsigned i = 0; i < _includes.size(); ++i) {
      qi::Path p(_includes.at(i));
      p /= pkgPath;
      if (p.isDir())
      {
        StringVector retfile;
        StringVector retdir;
        bool b = locateFileInDir(p, &retfile, &retdir);
        if (b) {
          qiLogVerbose() << "Found pkg '" << pkgName << "'";
          return retfile;
        }
      }
    }
    return StringVector();
  }

  bool PackageManager::hasError() const
  {
    PackagePtrMap::const_iterator it;
    for (it = _packages.begin(); it != _packages.end(); ++it) {
      if (it->second->hasError())
        return true;
    }
    return false;
  }

  void PackageManager::printMessage(std::ostream &os) const
  {
    PackagePtrMap::const_iterator it;
    for (it = _packages.begin(); it != _packages.end(); ++it) {
      it->second->printMessage(os);
    }
  }


  /**
   * @brief PackageManager::parsePackage
   * @param package
   *
   * locate the package... use QIPATH and -I to look for the package.
   * a folder with a "pkgname".pkg.qi file
   */
  void PackageManager::parsePackage(const std::string& packageName) {
    addPackage(packageName);
    PackagePtr pkg = package(packageName);
    if (pkg && pkg->_parsed) {
      qiLogVerbose() << "skipping pkg '" << packageName << "': already parsed";
      return;
    }
    StringVector sv = locatePackage(packageName);

    for (unsigned i = 0; i < sv.size(); ++i) {
      _parseFile(newFileReader(sv.at(i)));
    }

    // for each decl in the package. reference it into the package.
    ParseResultMap::iterator it;
    for (it = pkg->_contents.begin(); it != pkg->_contents.end(); ++it) {
      qiLogVerbose() << "Visiting: " << it->first;
      visitNode(it->second.ast, boost::bind<void>(&importExportDeclVisitor, _1, _2, boost::ref(pkg)));
    }
    qiLogVerbose() << "parsed pkg '" << packageName << "'";
    pkg->_parsed = true;
    pkg->dump();
  }

  void PackageManager::parseDir(const std::string& dirname)
  {
    qiLogVerbose() << "parsing dir: " << dirname;
    qi::Path fsp(dirname);
    if (!fsp.isDir())
      throw std::runtime_error(dirname + " is not a directory");

    StringVector resdir;
    StringVector resfile;
    locateFileInDir(dirname, &resfile, &resdir);
    for (unsigned i = 0; i < resfile.size(); ++i)
      _parseFile(newFileReader(resfile.at(i)));
    for (unsigned i = 0; i < resdir.size(); ++i)
      parseDir(dirname + "/" + resdir.at(i));
  }

  void PackageManager::parse(const std::string &fileOrPkg)
  {
    qi::Path fsp(fileOrPkg);
    if (fsp.isRegularFile()) {
      parseFile(newFileReader(fileOrPkg));
      return;
    }
    if (fsp.isDir()) {
      parseDir(fileOrPkg);
      return;
    }
    parsePackage(fileOrPkg);
  }

  /** parse all dependents packages
   */
  void PackageManager::resolvePackage(const std::string& packageName) {
    PackagePtr pkg = package(packageName);

    MessageVector mv;
    ASTMap::iterator it;
    for (it = pkg->_imports.begin(); it != pkg->_imports.end(); ++it) {

      try {
        //throw on error? should..
        parsePackage(it->first);
      } catch (const std::exception& e) {
        mv.push_back(Message(MessageType_Error, "Can't find package '" + it->first + "'"));//, it->second->loc()));
      }
    }

    //for each imports verify each symbol are correct
    for (it = pkg->_imports.begin(); it != pkg->_imports.end(); ++it) {
    }

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
    if (!packageName.empty()) {
      qiLogVerbose() << "Package Verification pkg:" << packageName;
      parsePackage(packageName);
      resolvePackage(packageName);
    }
    else {
      PackagePtrMap::iterator it;
      for (it = _packages.begin(); it != _packages.end(); ++it) {
        anal(it->first);
      }
    }
  }

  bool Package::hasError() const
  {
    ParseResultMap::const_iterator it;
    for (it = _contents.begin(); it != _contents.end(); ++it) {
      if (it->second.hasError())
        return true;
    }
    return false;
  }

  void Package::printMessage(std::ostream &os) const
  {
    ParseResultMap::const_iterator it;
    for (it = _contents.begin(); it != _contents.end(); ++it) {
      it->second.printMessage(os);
    }
  }

};
