

#include <iostream>
#include <qimessaging/applicationsession.hpp>
#include <qi/log.hpp>
#include <fstream>
#include <qilang/node.hpp>
#include <qilang/parser.hpp>
#include <qilang/formatter.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

typedef std::vector<std::string> StringVector;



int main(int argc, char *argv[])
{
  qi::ApplicationSession app(argc, argv);

  po::options_description desc("qirtidl options");
  desc.add_options()
      ("help,h", "this help")
      ("codegen,c", po::value<std::string>(), "code generator")
      ("services,s", po::value<std::vector<std::string> >(), "list of services")
      ;

  po::positional_options_description p;
  p.add("services", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  std::ostream*            out;
  std::ofstream            of;
  std::string codegen = vm["codegen"].as<std::string>();

  if (vm.count("output-file")) {
    std::string outf = vm["output-file"].as<std::string>();
    of.open(outf.c_str());
    out = &of;
  } else {
    out = &std::cout;
  }

  app.start();

  std::vector<std::string> services;
  if (vm.count("services"))
  {
    services = vm["services"].as<std::vector<std::string> >();
  }
  else{
    std::vector< qi::ServiceInfo> si;
    si = app.session()->services();
    for ( unsigned i = 0; i < si.size(); ++i) {
      services.push_back(si.at(i).name());
    }
  }

  for (unsigned i = 0; i < services.size(); ++i) {
    qi::AnyObject obj = app.session()->service(services.at(i));

    const qi::MetaObject& mo = obj.metaObject();
    qilang::NodePtrVector objs;
    objs.push_back(qilang::metaObjectToQiLang(services.at(i), mo));

    bool succ = qilang::codegen(codegen, pm, pr);
    if (!succ)
      return 1;
  }
 return 0;
}
