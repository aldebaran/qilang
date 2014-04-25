/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <iostream>
#include <qi/log.hpp>
#include <fstream>
#include <qilang/node.hpp>
#include <qilang/parser.hpp>
#include <qilang/formatter.hpp>
#include <qilang/packagemanager.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
  po::options_description desc("qilang options");
  desc.add_options()
      ("help,h", "produce help message")
      ("codegen,c", po::value<std::string>()->default_value(""), "Set the codegenerator to use")
      ("input-file", po::value< std::vector< std::string> >(), "input files")
      ("include,I", po::value< std::vector< std::string> >(), "include directories for packages")
      ("output-file,o", po::value<std::string>(), "output dir")
      ;

  po::positional_options_description p;
  p.add("input-file", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
  po::notify(vm);


  if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 1;
  }

  std::ostream*            out;
  std::string              codegen;
  std::vector<std::string> files;
  std::vector<std::string> includes;
  std::ofstream            of;

  codegen = vm["codegen"].as<std::string>();
  //if (codegen != "cppr" && codegen != "cppi" && codegen != "qilang" && codegen != "sexpr") {
  //  std::cout << "Invalid codegen value: use cpp/qilang/sexpr" << std::endl;
  //  exit(1);
 // }

  if (vm.count("output-file")) {
    std::string outf = vm["output-file"].as<std::string>();
    of.open(outf.c_str());
    out = &of;
  } else {
    out = &std::cout;
  }


  qilang::PackageManager pm;

  if (vm.count("include"))
    includes = vm["include"].as< std::vector<std::string> >();

  pm.setIncludes(includes);

  files = vm["input-file"].as< std::vector<std::string> >();
  for (int i = 0; i < files.size(); ++i) {
    std::cout << "Generating " << codegen << " for " << files.at(i) << std::endl;

    qilang::NodePtrVector rootnode;
    try {
      rootnode = pm.parseFile(files.at(i));
    } catch(const std::exception& e) {
      std::cout << e.what() << std::endl;
      exit(1);
    }

    if (codegen == "cppi")
      *out << qilang::genCppObjectInterface(rootnode);
    else if (codegen == "cppr")
      *out << qilang::genCppObjectRegistration(rootnode);
    else if (codegen == "qilang")
      *out << qilang::format(rootnode);
    else if (codegen == "sexpr")
      *out << qilang::formatAST(rootnode);
      //qilang::PackagePtr p = pm.package("foo");
      //p->dump();
    }
  if (codegen == "testgros") {
    pm.anal("pimpl");
  }
  of.close();
  return 0;
}
