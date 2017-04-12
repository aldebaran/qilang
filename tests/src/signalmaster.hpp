#ifndef TESTQILANG_SIGNALMASTERIMPL_HPP
#define TESTQILANG_SIGNALMASTERIMPL_HPP

#include <src/somesignals_p.hpp>

namespace testqilang
{
struct SignalMasterImpl
{
  qi::Signal<void> voidSignal;
  qi::Signal<int> intSignal;
};
} // testqilang

QI_REGISTER_IMPLEMENTATION_H(testqilang::SignalMaster, testqilang::SignalMasterImpl)

#endif // TESTQILANG_SIGNALMASTERIMPL_HPP
