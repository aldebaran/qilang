#ifndef TESTQILANG_OUROBOROSIMPL_HPP
#define TESTQILANG_OUROBOROSIMPL_HPP

#include <src/testinterface_p.hpp>

namespace testqilang
{
class OuroborosImpl
{
public:
  void eatMyself(qi::Object<Ouroboros> o) {}
};
} // testqilang

QI_REGISTER_IMPLEMENTATION_H(testqilang::Ouroboros, testqilang::OuroborosImpl)

#endif // TESTQILANG_OUROBOROSIMPL_HPP
