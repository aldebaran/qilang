/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <iostream>
#include <qi/applicationsession.hpp>
#include <qi/log.hpp>
#include <qi/path_conf.hpp>
#include <fstream>
#include <qilang/node.hpp>
#include <qilang/parser.hpp>
#include <qilang/formatter.hpp>
#include <qilang/packagemanager.hpp>
#include <boost/program_options.hpp>
#include <qi/session.hpp>
#include <qilang/pathformatter.hpp>

qiLogCategory("qic");
namespace po = boost::program_options;

int codegen_service(const std::string& codegen,
                    qilang::FileWriterPtr out,
                    qilang::PackageManagerPtr pm,
                    qi::SessionPtr session,
                    const std::string& service) {
  qiLogVerbose() << "Generation " << codegen << " for service " << service;

  qi::AnyObject obj = session->service(service).value();

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
    std::cerr << "Exception: " << e.what() << std::endl;
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

  bool help = false;
  std::string codegen;
  std::string mode;
  std::string idlFile = "";
  boost::optional<std::string> outputFile;
  boost::optional<std::string> targetSdkDir;
  std::vector<std::string> importDirs;
  po::options_description desc("qilang options");
  desc.add_options()
      ("help,h", po::bool_switch(&help), "produce help message")
      ("codegen,c", po::value(&codegen)->required(), "Set the codegenerator to use")
      ("input-mode,i", po::value(&mode)->default_value("file"), "Set the input type (file or service)")
      ("input", po::value(&idlFile)->required(), "input file")
      ("output-file,o", po::value(&outputFile), "output file")
      ("target-sdk-dir,t", po::value(&targetSdkDir), "the SDK directory of the target platform")
      (",I", po::value(&importDirs)->composing(), "add a directory to be searched for imported packages")
      ;

  po::positional_options_description p;
  p.add("input", 1);

  try {
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (help) {
        std::cout << desc << std::endl;
        return 1;
    }
    if (targetSdkDir) {
      auto path = qi::Path::fromNative(qilang::formatPath(*targetSdkDir));
      if (!path.isEmpty()) {
        // first add the "${target-sdk-dir}"
        pm->addLookupPaths({path.str()});
        // then add paths which are listed in "${target-sdk-dir}/share/qi/path.conf"
        pm->addLookupPaths(qi::path::parseQiPathConf(path.str()));
      }
    }

    for (auto& importDir : importDirs) {
      importDir = qilang::formatPath(importDir);
    }
    pm->addLookupPaths(importDirs);

    qilang::FileWriterPtr out;

    if (outputFile) {
      std::string outf = qilang::formatPath(*outputFile);
      out = qilang::newFileWriter(outf);
    } else {
      out = qilang::newFileWriter(&std::cout, "cout");
    }

    idlFile = qilang::formatPath(idlFile);

    if (mode == "service") {
      app.startSession();
      return codegen_service(codegen, out, pm, app.session(), idlFile);
    } else if (mode == "file") {
      return codegen_file(codegen, out, pm, idlFile);
    } else {
      throw std::runtime_error("bad input option value. must be service or file");
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception:" << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown exception" << std::endl;
    return 1;
  }
  return 0;
}
