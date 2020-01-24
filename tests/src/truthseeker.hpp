#ifndef TESTQILANG_TRUTHSEEKER_HPP
#define TESTQILANG_TRUTHSEEKER_HPP

#include <src/someinterfaces_p.hpp>
#include <testqilang/somemix.hpp>
#include <thread>
#include <chrono>

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

namespace testqilang
{
class FooImpl
{
public:
  // For context, see test "QiLangFunction.callsAreSafeWhenDestroyed".
  int accessMember(qi::AnyObject fut)
  {
    auto futDestroyed = _promiseDestroyed.future();
    qiLogInfo("FooImpl") << "Entered call, waiting ...";
    fut.call<void>("value", 0); // Only continue once the test has lost its reference to this object.
    qiLogInfo("FooImpl") << "Finishing call.";
    if (futDestroyed.isFinished())
      throw std::runtime_error("Member function called while `this` is destroyed.");
    return 42;
  }

  qi::Future<void> onDestroyed() const
  {
    return _promiseDestroyed.future();
  }

  ~FooImpl()
  {
    _promiseDestroyed.setValue(nullptr);
  }

private:
  qi::Promise<void> _promiseDestroyed;
};
}

QI_REGISTER_IMPLEMENTATION_H(testqilang::Foo, testqilang::FooImpl)

#endif // TESTQILANG_TRUTHSEEKER_HPP
