#ifndef TESTQILANG_KINDAMANAGERIMPL_HPP
#define TESTQILANG_KINDAMANAGERIMPL_HPP

#include <src/somemix_p.hpp>
#include <qi/clock.hpp>

namespace testqilang
{
class KindaManagerImpl
{
public:
  qi::Future<int> findTruth()
  {
    return qi::Future<int>(42);
  }

  qi::Future<qi::SystemClockTimePoint> whatsTheTime()
  {
    return qi::Future<qi::SystemClockTimePoint>(qi::SystemClock::now());
  }

  Option setOption(Option opt)
  {
    return opt;
  }

  qi::Signal<float> test;
  qi::Property<std::vector<float>> current;
};
} // testqilang

QI_REGISTER_IMPLEMENTATION_H(testqilang::KindaManager, testqilang::KindaManagerImpl)

#endif // TESTQILANG_KINDAMANAGERIMPL_HPP
