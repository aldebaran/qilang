/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <iostream>
#include <qi/applicationsession.hpp>
#include <qi/log.hpp>
#include <fstream>
#include <qilang/node.hpp>
#include <qilang/parser.hpp>
#include <qilang/formatter.hpp>
#include <qilang/packagemanager.hpp>
#include <boost/program_options.hpp>
#include <qi/session.hpp>

qiLogCategory("qic");
namespace po = boost::program_options;

int codegen_service(const std::string& codegen,
                    qilang::FileWriterPtr out,
                    qilang::PackageManagerPtr pm,
                    qi::SessionPtr session,
                    const std::string& service) {
  qiLogVerbose() << "Generation " << codegen << " for service " << service;

  qi::AnyObject obj = session->service(service);

  const qi::MetaObject& mo = obj.metaObject();
  qilang::NodePtrVector objs;
  objs.push_back(qilang::metaObjectToQiLang(service, mo));

  qilang::ParseResultPtr pr = qilang::newParseResult();
  pr->ast = objs;

  bool succ = qilang::codegen(out, codegen, pm, pr);
  if (!succ)
    return 1;
  return 0;
}



int codegen_file(const std::string& codegen,
                 qilang::FileWriterPtr out,
                 qilang::PackageManagerPtr pm,
                 const std::string& file) {
  qiLogVerbose() << "Generating " << codegen << " for file " << file;
  qilang::ParseResultPtr pr;
  try {
    pr = pm->parseFile(qilang::newFileReader(file));
  } catch(const std::exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
    exit(1);
  }
  bool ret = qilang::codegen(out, codegen, pm, pr);
  if (!ret)
    return 1;
  return 0;
}

int main(int argc, char *argv[])
{
  qi::ApplicationSession app(argc, argv);
  qilang::PackageManagerPtr pm = qilang::newPackageManager();

  po::options_description desc("qilang options");
  desc.add_options()
      ("help,h", "produce help message")
      ("codegen,c", po::value<std::string>(), "Set the codegenerator to use")
      ("input-mode,i", po::value<std::string>()->default_value("file"), "Set the input type (file or service)")
      ("inputs", po::value< std::vector< std::string> >(), "input files")
      ("include,I", po::value< std::vector< std::string> >(), "include directories for packages")
      ("output-file,o", po::value<std::string>(), "output file")
      ;

  po::positional_options_description p;
  p.add("inputs", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 1;
  }

  qilang::FileWriterPtr    out;
  std::string              codegen = vm["codegen"].as<std::string>();
  std::string              mode = vm["input-mode"].as<std::string>();
  std::vector<std::string> inputs;
  std::vector<std::string> includes;

  if (vm.count("inputs"))
    inputs = vm["inputs"].as<std::vector<std::string> >();

  if (vm.count("output-file")) {
    std::string outf = vm["output-file"].as<std::string>();
    out = qilang::newFileWriter(outf);
  } else {
    out = qilang::newFileWriter(&std::cout, "cout");
  }

  if (vm.count("include"))
    includes = vm["include"].as< std::vector<std::string> >();

  pm->setIncludes(includes);

  if (mode == "service") {
    app.start();
    return codegen_service(codegen, out, pm, app.session(), inputs[0]);
  } else if (mode == "file") {
    return codegen_file(codegen, out, pm, inputs[0]);
  } else {
    throw std::runtime_error("bad input option value. must be service or file");
  }

  return 0;
}
