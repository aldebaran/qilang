#ifndef TESTQILANG_BUFFY_HPP
#define TESTQILANG_BUFFY_HPP

#include <src/buffy_p.hpp>

namespace testqilang
{
struct BuffyImpl
{
  Vampire fightAgainst(const Vampire& v) { return v; }
  qi::Buffer bufferIdentity(const qi::Buffer& b) { return b; }
};
} // testqilang

QI_REGISTER_IMPLEMENTATION_H(testqilang::Buffy, testqilang::BuffyImpl)

#endif // TESTQILANG_BUFFY_HPP
