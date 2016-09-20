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
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <qi/qi.hpp>
#include <qi/path.hpp>

qiLogCategory("qilang.pm");

namespace qilang {

  std::string extractPackageName(const NodePtr& node) {
    switch(node->type()) {
      case NodeType_Package: {
        PackageNode* tnode = static_cast<PackageNode*>(node.get());
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
      // EXPORT
      case NodeType_InterfaceDecl: {
        InterfaceDeclNode* tnode = static_cast<InterfaceDeclNode*>(node.get());
        pkg->addMember(tnode->name, node);
        return;
      } case NodeType_StructDecl: {
        StructDeclNode* tnode = static_cast<StructDeclNode*>(node.get());
        pkg->addMember(tnode->name, node);
        return;
      } case NodeType_FnDecl: {
        FnDeclNode* tnode = static_cast<FnDeclNode*>(node.get());
        pkg->addMember(tnode->name, node);
        return;
      } case NodeType_ConstDecl: {
        ConstDeclNode* tnode = static_cast<ConstDeclNode*>(node.get());
        pkg->addMember(tnode->name, node);
        return;
      } case NodeType_ObjectDef: {
        ObjectDefNode* tnode = static_cast<ObjectDefNode*>(node.get());
        pkg->addMember(tnode->name, node);
        return;
      } case NodeType_TypeDefDecl: {
        TypeDefDeclNode* tnode = static_cast<TypeDefDeclNode*>(node.get());
        pkg->addMember(tnode->name, node);
        return;
      } case NodeType_EnumDecl: {
        EnumDeclNode* tnode = static_cast<EnumDeclNode*>(node.get());
        pkg->addMember(tnode->name, node);
        return;
      }

      // IMPORT
      case NodeType_Import: {
        ImportNode* tnode = static_cast<ImportNode*>(node.get());
        pkg->addImport(tnode->name, node);
        return;
      }
      default: {
        return;
      }
    }
  }

  PackagePtr    PackageManager::package(const std::string& packagename) const {
    PackagePtrMap::const_iterator it;
    it = _packages.find(packagename);
    if (it == _packages.end())
      throw std::runtime_error("package not found '" + packagename + "'");
    return it->second;
  }

  /** 1 / Check for missing or multiple package declaration
   *  2 / Check that the directory path and package name match
   *  3 / register the content of the file to the package
   */
  bool PackageManager::addFileToPackage(const std::string& absfile, const FileReaderPtr& file, ParseResultPtr& pr) {

    // 1
    NodePtrVector result;
    result = findNode(pr->ast, NodeType_Package);
    if (result.size() == 0) {
      pr->addDiag(Diagnostic(DiagnosticType_Error, "missing package declaration", Location(file->filename())));
      return false;
    }
    if (result.size() > 1) {
      for (unsigned i = 1; i < result.size(); ++i) {
        pr->addDiag(Diagnostic(DiagnosticType_Error, "extra package declaration", result.at(i)->loc()));

      }
      pr->addDiag(Diagnostic(DiagnosticType_Info, "previous declared here", result.at(0)->loc()));
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

    qi::Path p = pf.parent().absolute();
    for (int i = leafs.size() - 1; i >= 0; --i) {
      std::string par = p.filename();
      if (par != leafs.at(i)) {
        pr->addDiag(Diagnostic(DiagnosticType_Error,
                                          "package name '" + pkgname + "' do not match parent directory name '" + (std::string)dirname + "'",
                                          result.at(0)->loc()));
        return false;
      }
      p = p.parent();
    }

    std::string pkgpath = p.absolute().str();

    addInclude(pkgpath);
    addPackage(pkgname);
    pr->package = pkgname;
    package(pkgname)->setContent(absfile, pr);
    return true;
  }

  ParseResultPtr PackageManager::parseFile(const FileReaderPtr& file)
  {
    qi::Path fsfname = qi::Path(file->filename());
    std::string filename = fsfname.absolute().str();
    if (!fsfname.isRegularFile())
      throw std::runtime_error(file->filename() + " is not a regular file");
    qiLogVerbose() << "Parsing file: " << filename;
    if (_sources.find(filename) != _sources.end()) {
      qiLogVerbose() << "already parsed, skipping '" << filename << "'";
      return package(_sources[filename])->_contents[filename];
    }

    ParseResultPtr ret = qilang::parse(file);
    if (addFileToPackage(filename, file, ret))
      _sources[filename] = ret->package;
    return ret;
  }


  static bool locateFileInDir(const std::string& path, std::unordered_set<std::string>* resultfile, StringVector* resultdir) {
    qi::PathVector pv = qi::Path(path).dirs();
    bool ret = false;
    for (unsigned i = 0; i < pv.size(); ++i) {
      qi::Path& p = pv.at(i);
      resultdir->push_back(p.str());
    }

    pv = qi::Path(path).files();
    for (unsigned i = 0; i < pv.size(); ++i) {
      qi::Path& p = pv.at(i);
      if (p.isRegularFile()) {
        if (p.extension() == ".qi") {
          resultfile->insert(p.bfsPath().generic_string());
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

  std::unordered_set<std::string> PackageManager::locatePackage(const std::string& pkgName) {

    if (pkgName.empty())
      throw std::runtime_error("empty package name");

    qi::Path pkgPath(pkgNameToDir(pkgName));

    std::unordered_set<std::string> packageFiles;
    using boost::filesystem::recursive_directory_iterator;
    recursive_directory_iterator itPath, itEnd;
    for (qi::Path lookupPath : _lookupPaths) {
      lookupPath /= "share/qi/idl";
      lookupPath /= pkgPath;
      if (!lookupPath.exists()) {
        continue;
      }
      for (itPath = recursive_directory_iterator(lookupPath.bfsPath());
           itPath != itEnd; ++itPath) {
        auto path = itPath->path();
        if (boost::algorithm::ends_with(path.string(), ".idl.qi")) {
          packageFiles.insert(path.string(qi::unicodeFacet()));
          qiLogVerbose() << "Found package '" << pkgName << "' in " << path;
        }
      }
    }
    return packageFiles;
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
    for (PackagePtrMap::const_iterator it = _packages.begin(); it != _packages.end(); ++it)
      it->second->printMessage(os);
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
    auto sv = locatePackage(packageName);

    for (const auto& currSv : sv) {
      parseFile(newFileReader(currSv));
    }

    // for each decl in the package. reference it into the package.
    ParseResultMap::iterator it;
    for (it = pkg->_contents.begin(); it != pkg->_contents.end(); ++it) {
      qiLogVerbose() << "Visiting: " << it->first;
      visitNode(it->second->ast, boost::bind<void>(&importExportDeclVisitor, _1, _2, boost::ref(pkg)));
    }
    qiLogVerbose() << "parsed pkg '" << packageName << "'";
    pkg->_parsed = true;
  }

  void PackageManager::parseDir(const std::string& dirname)
  {
    qiLogVerbose() << "parsing dir: " << dirname;
    qi::Path fsp(dirname);
    if (!fsp.isDir())
      throw std::runtime_error(dirname + " is not a directory");

    StringVector resdir;
    std::unordered_set<std::string> resfile;
    locateFileInDir(dirname, &resfile, &resdir);
    for (const auto& currResfile : resfile)
      parseFile(newFileReader(currResfile));
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

  //throw on error
  static ResolutionResult checkImport(const PackageManager& pm, const ParseResultPtr& pr, const std::string& pkgName, const CustomTypeExprNode* tnode, const std::string& type)
  {
    PackagePtr pkg = pm.package(pkgName);
    NodePtr node = pkg->getExport(type);
    TypeKind kind;
    if (node) {
      switch (node->type()) {
      case NodeType_InterfaceDecl:
        kind = TypeKind_Interface;
        break;
      case NodeType_EnumDecl:
        kind = TypeKind_Enum;
        break;
      case NodeType_StructDecl:
        kind = TypeKind_Struct;
        break;
      default:
        pr->addDiag(Diagnostic(DiagnosticType_Error, "'" + type + "' in package '" + pkgName + "' is not a type", tnode->loc()));
        throw std::runtime_error("Not a type");
        break;
      }
      return ResolutionResult(pkgName, type, kind);
    }
    pr->addDiag(Diagnostic(DiagnosticType_Error, "Can't find '" + type + "' in package '" + pkgName + "'", tnode->loc()));
    throw std::runtime_error("Can't find import");
  }

  //throw on error
  ResolutionResult PackageManager::resolveImport(const ParseResultPtr& pr, const PackagePtr& pkg, const CustomTypeExprNode* tnode)
  {
    const std::string type = tnode->value;
    qiLogVerbose() << "Resolving: " << type << " from package: " << pkg->_name;
    const auto lastDot = type.find_last_of('.');
    const std::string pkgName = (lastDot == std::string::npos) ? "" : type.substr(0, lastDot);
    const auto valueBegins = pkgName.empty() ? 0 : (pkgName.size() + 1);
    std::string value = type.substr(valueBegins);

    //package name provided
    if (!pkgName.empty() && pkgName != type)
      return checkImport(*this, pr, pkgName, tnode, value);
    value = type;

    //no package name. find the package name
    NodePtr exportnode = pkg->getExport(type);
    if (exportnode)
      return checkImport(*this, pr, pkg->_name, tnode, type);

    ASTMap::const_iterator it;
    for (it = pkg->_imports.begin(); it != pkg->_imports.end(); ++it) {
      const NodePtrVector& v = it->second;
      for (unsigned i = 0; i < v.size(); ++i) {
        ImportNode* inode = static_cast<ImportNode*>(v.at(i).get());
        switch (inode->importType) {
          case ImportType_All: {
            return checkImport(*this, pr, inode->name, tnode, type);
          }
          case ImportType_List: {
            StringVector::iterator it = std::find(inode->imports.begin(), inode->imports.end(), type);
            if (it != inode->imports.end()) {
              return checkImport(*this, pr, inode->name, tnode, type);
            }
            break;
          }
          case ImportType_Package:
            break;
        }
      }
    }
    pr->addDiag(Diagnostic(DiagnosticType_Error, "cant resolve id '" + type + "' from package '" + pkg->_name + "'", tnode->loc()));
    throw std::runtime_error("cant resolve id");
  }

  /** parse all dependents packages
   */
  void PackageManager::resolvePackage(const std::string& packageName) {
    PackagePtr pkg = package(packageName);

    DiagnosticVector mv;
    ASTMap::iterator it;
    for (it = pkg->_imports.begin(); it != pkg->_imports.end(); ++it) {

      try {
        //throw on error? should..
        parsePackage(it->first);
      } catch (const std::exception& e) {
        mv.push_back(Diagnostic(DiagnosticType_Error, "Can't find package '" + it->first + "'"));//, it->second->loc()));
      }
    }

    //for each imports verify each symbol are correct
    //for (it = pkg->_imports.begin(); it != pkg->_imports.end(); ++it) {
    //}

    //for each customtype expr resolve name
    //for each files in the package
    ParseResultMap::iterator it2;
    for (it2 = pkg->_contents.begin(); it2 != pkg->_contents.end(); ++it2) {
      NodePtrVector customs = findNode(it2->second->ast, NodeType_CustomTypeExpr);

      for (unsigned j = 0; j < customs.size(); ++j) {
        CustomTypeExprNode* tnode = static_cast<CustomTypeExprNode*>(customs.at(j).get());
        ResolutionResult sp;
        try {
          sp = resolveImport(it2->second, pkg, tnode);
        } catch(const std::exception& e) {
          it2->second->addDiag(Diagnostic(DiagnosticType_Error, "Can't find id '" + tnode->value + "'", tnode->loc()));
          continue;
        }
        qiLogVerbose() << "resolved value '" << tnode->value << " to '" << sp.pkg << "." << sp.type << "'";
        tnode->resolved_package = sp.pkg;
        tnode->resolved_value   = sp.type;
        tnode->resolved_kind    = sp.kind;
      }
    }
  }

  void PackageManager::addLookupPaths(const StringVector& lookupPaths) {
    _lookupPaths.reserve(_lookupPaths.size() + lookupPaths.size());
    for (const auto& path : lookupPaths) {
      _lookupPaths.push_back(path);
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
      if (it->second->hasError())
        return true;
    }
    return false;
  }

  void Package::printMessage(std::ostream &os) const
  {
    ParseResultMap::const_iterator it;
    for (it = _contents.begin(); it != _contents.end(); ++it) {
      it->second->printMessage(os);
    }
  }

};
