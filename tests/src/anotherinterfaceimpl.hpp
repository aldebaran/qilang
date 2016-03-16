#include <src/somemix_p.hpp>
#include <qi/clock.hpp>
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

  qi::Signal<qi::ClockTimePoint> blop;
  qi::Property<std::vector<std::string>> crap;
};
} // testqilang

QI_REGISTER_IMPLEMENTATION_H(testqilang::AnotherInterface, testqilang::AnotherInterfaceImpl)
