/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <iostream>
#include <qilang/formatter.hpp>
#include <qilang/packagemanager.hpp>

qiLogCategory("qilang.codegen");

namespace qilang {

  bool codegen(const FileWriterPtr&             out,
               const std::string&               codegen,
               const qilang::PackageManagerPtr& pm,
               const qilang::ParseResultPtr&    pr)
  {
    static const char* vals[] = { "cpp_interface", "cppi",
                                  "cpp_local", "cppl",
                                  "cpp_remote", "cppr",
                                  "sexpr", "qilang", "doc", 0 };
    int index = 0;
    const char* v = vals[index];
    while (v) {
      if (codegen == v)
        break;
      index++;
      v = vals[index];
    }
    if (!v)
      throw std::runtime_error("bad value for codegen");

    if (pr->hasError()) {
      return false;
    }
    if (codegen == "qilang") {
      out->out() << qilang::format(pr->ast);
      return true;
    }
    else if (codegen == "sexpr") {
      out->out() << qilang::formatAST(pr->ast);
      return true;
    }
    else if (codegen == "doc") {
      out->out() << qilang::genDoc(pr->ast);
      return true;
    }
    pm->anal();
    if (pm->hasError()) {
      return false;
    }
    if      (codegen == "cpp_interface" || codegen == "cppi")
      out->out() << qilang::genCppObjectInterface(pm, pr);
    else if (codegen == "cpp_local"     || codegen == "cppl")
      out->out() << qilang::genCppObjectLocal(pm, pr);
    else if (codegen == "cpp_remote"    || codegen == "cppr")
      out->out() << qilang::genCppObjectRemote(pm, pr);
    return true;
  }

}
