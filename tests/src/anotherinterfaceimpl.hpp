#include <src/somemix_p.hpp>
#include "kindamanagerimpl.hpp"

namespace testqilang
{
class AnotherInterfaceImpl
{
public:
  KindaManagerPtr makeSomething()
  {
    return boost::make_shared<KindaManagerImpl>();
  }

  qi::Signal<bool> blop;
  qi::Property<std::vector<std::string>> crap;
};
} // testqilang

QI_REGISTER_IMPLEMENTATION_H(testqilang::AnotherInterface, testqilang::AnotherInterfaceImpl)
