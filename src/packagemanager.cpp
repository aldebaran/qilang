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

namespace qilang {

  void packageVisitor(const NodePtr& parent, const NodePtr& node, qilang::StringVector& result) {
    switch(node->type()) {
      case NodeType_Package: {
        PackageNode* tnode = dynamic_cast<PackageNode*>(node.get());
        result.push_back(tnode->name);
        qiLogInfo() << "found pkg:" << tnode->name;
        break;
      } default:
        break;
    };
  }

  void exportedDeclVisitor(const NodePtr& parent, const NodePtr& node, PackagePtr& pkg) {
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
      } default: {
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

  NodePtrVector PackageManager::parseFile(const std::string& filename) {
    NodePtrVector ret = qilang::parse(filename);
    _sources[filename] = ret;

    StringVector sv;
    visitNode(ret, boost::bind<void>(&packageVisitor, _1, _2, boost::ref(sv)));

    if (sv.size() != 1)
      throw std::runtime_error("0 or >1 package definition");
    std::string pkgname = sv[0];

    addPackage(pkgname);
    package(pkgname)->setContent(filename, ret);
    return ret;
  }


  static void findFileOfPackage(const std::string& package) {
  }

  /**
   * @brief PackageManager::parsePackage
   * @param package
   *
   * locate the package... use QIPATH and -I to look for the package.
   * a folder with a "pkgname".pkg.qi file
   */
  void PackageManager::parsePackage(const std::string& package) {
    // locate the package...
  }

  void PackageManager::anal(const std::string &packageName) {
    qiLogVerbose() << "Analysing pkg:" << packageName;
    // for each decl in the package. reference it into the package.
    PackagePtr pkg = package(packageName);

    ASTMap::iterator it;
    for (it = pkg->_contents.begin(); it != pkg->_contents.end(); ++it) {
      qiLogVerbose() << "Visiting: " << it->first;
      visitNode(it->second, boost::bind<void>(&exportedDeclVisitor, _1, _2, boost::ref(pkg)));
    }
    pkg->dump();
  }

};
