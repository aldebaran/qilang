#include <gtest/gtest.h>
#include <qi/anymodule.hpp>
#include <testsession/testsession.hpp>
#include <testqilang/somemix.hpp>
#include <testqilang/somestructs.hpp>
#include <testqilang/someinterfaces.hpp>
#include <qi/testutils/testutils.hpp>

class QiLangFunction: public ::testing::Test
{
protected:
  static void SetUpTestCase()
  {
    _testqilang = qi::import("testqilang_module");
  }

  static void TearDownTestCase()
  {
    _testqilang.reset();
  }

  static qi::AnyModule _testqilang;
};

qi::AnyModule QiLangFunction::_testqilang{};

using namespace testqilang;

TEST_F(QiLangFunction, OverloadedMethodsWithDefaultArguments)
{
  auto km = _testqilang.call<KindaManagerPtr>("KindaManager");
  km->overlord();
  km->overlord("I'm gonna send him to out of space to find another race");
}

TEST_F(QiLangFunction, OverloadedMethodsWithDifferentDeclarations)
{
  auto km = _testqilang.call<KindaManagerPtr>("KindaManager");
  km->overlord();
  km->overlord(42);
}

TEST_F(QiLangFunction, MethodOfAnActor)
{
  _testqilang.call<BradPittPtr>("BradPitt")->act();
}

TEST_F(QiLangFunction, callsAreSafeWhenDestroyed)
{
  // See `testqilang::FooImpl` for details.
  // We call an object, make it stop in the middle of the call,
  // then lose all references to the object and continue the execution of the call.
  // The object must still be maintained alive until the call is finished.
  qi::Promise<void> promiseFinishCall;
  qi::Future<void> futObjectIsDestroyed;
  qi::Future<int> futCall;
  {
    auto obj = _testqilang.call<qi::AnyObject>("Foo");
    futObjectIsDestroyed = obj.async<void>("onDestroyed");
    qi::AnyObject futFinishCall(boost::make_shared<qi::Future<void>>(promiseFinishCall.future()));
    ASSERT_NO_THROW(futCall = obj.async<int>("accessMember", std::move(futFinishCall)));
  }
  promiseFinishCall.setValue(nullptr); // Execution of the call will wait for this promise to be set.
  ASSERT_TRUE(test::finishesWithValue(futObjectIsDestroyed)); // Wait until the object is really destroyed.
  ASSERT_TRUE(test::finishesWithValue(futCall));
}
