#ifndef TESTQILANG_TRUTHSEEKER_HPP
#define TESTQILANG_TRUTHSEEKER_HPP

#include <src/someinterfaces_p.hpp>
#include <testqilang/somemix.hpp>

namespace testqilang
{
class TruthSeekerImpl
{
public:
  int answer() { return 42; }
  int bradPittAnswer(testqilang::BradPittPtr) { return 43; }
};
} // testqilang

QI_REGISTER_IMPLEMENTATION_H(testqilang::TruthSeeker, testqilang::TruthSeekerImpl)

#endif // TESTQILANG_TRUTHSEEKER_HPP
