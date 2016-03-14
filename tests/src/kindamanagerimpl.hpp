#ifndef TESTQILANG_KINDAMANAGERIMPL_HPP
#define TESTQILANG_KINDAMANAGERIMPL_HPP

#include <src/somemix_p.hpp>

namespace testqilang
{
class KindaManagerImpl
{
public:
  qi::Future<int> findTruth()
  {
    return qi::Future<int>(42);
  }

  qi::Signal<float> test;
  qi::Property<std::vector<float>> current;
};
} // testqilang

QI_REGISTER_IMPLEMENTATION_H(testqilang::KindaManager, testqilang::KindaManagerImpl)

#endif // TESTQILANG_KINDAMANAGERIMPL_HPP
