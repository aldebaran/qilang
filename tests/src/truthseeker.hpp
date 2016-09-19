#ifndef TESTQILANG_TRUTHSEEKER_HPP
#define TESTQILANG_TRUTHSEEKER_HPP

#include <src/someinterfaces_p.hpp>

namespace testqilang
{
class TruthSeekerImpl
{
public:
  int answer() { return 42; }
};
} // testqilang

QI_REGISTER_IMPLEMENTATION_H(testqilang::TruthSeeker, testqilang::TruthSeekerImpl)

#endif // TESTQILANG_TRUTHSEEKER_HPP
