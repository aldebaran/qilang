#ifndef TESTQILANG_BRADPITTIMPL_HPP
#define TESTQILANG_BRADPITTIMPL_HPP
#include <src/somemix_p.hpp>
#include <tests/test_qilang_test_utils.hpp>

namespace testqilang
{
  class BradPittImpl
    : public qi::Actor
  {
    test::qilang::ChecksConcurrentCalls checker{ typeid(BradPittImpl).name() };
  public:

    qi::Future<void> act()
    {
      // We want to check that no call to this function is ever
      // done concurrently (because the caller should automatically
      // handle that this type inherits from qi::Actor).
      if (auto maybeErrorMessage = checker.failOnConcurrentCalls())
        return qi::makeFutureError<void>(*maybeErrorMessage);
      else
        return qi::Future<void>{ nullptr };
    }
  };

} // testqilang

QI_REGISTER_IMPLEMENTATION_H(testqilang::BradPitt, testqilang::BradPittImpl)

#endif // TESTQILANG_BRADPITTIMPL_HPP
