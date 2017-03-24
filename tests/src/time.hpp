#ifndef TESTQILANG_TIME_HPP
#define TESTQILANG_TIME_HPP

#include <src/time_p.hpp>
#include <qi/clock.hpp>

namespace testqilang
{
struct TimeLordImpl
{
  qi::NanoSeconds nsecIdentity(qi::NanoSeconds t) { return t; }
  qi::MicroSeconds usecIdentity(qi::MicroSeconds t) { return t; }
  qi::MilliSeconds msecIdentity(qi::MilliSeconds t) { return t; }
  qi::Seconds secIdentity(qi::Seconds t) { return t; }
  qi::Minutes minIdentity(qi::Minutes t) { return t; }
  qi::Hours hourIdentity(qi::Hours t) { return t; }
};
} // testqilang

QI_REGISTER_IMPLEMENTATION_H(testqilang::TimeLord, testqilang::TimeLordImpl)

#endif // TESTQILANG_TIME_HPP
