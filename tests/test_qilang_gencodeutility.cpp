#include <gtest/gtest.h>

#include <thread>

#include <qi/clock.hpp>

#include <tests/test_qilang_test_utils.hpp>

#include <tests/qilang/gencodeutility.hpp>


namespace {

  const auto waitTimeout = qi::Seconds{ 5 };
  const auto checkDurationPerCall = std::chrono::milliseconds{ 500 };

  template<typename T>
  struct Base
  {
    std::vector<std::string> log;

    test::qilang::ChecksConcurrentCalls checker{ typeid(T).name() };

    void foo() { log.push_back("foo()"); }
    void foo(int, int, int) { log.push_back("foo(int, int, int)"); }

    qi::Future<void> asyncFoo()
    {
      auto maybeErrorMessage = checker.failOnConcurrentCalls(checkDurationPerCall);
      log.push_back("asyncFoo()");
      if (maybeErrorMessage)
        return qi::makeFutureError<void>(*maybeErrorMessage);
      else
        return qi::Future<void>{nullptr};
    }
  };

  struct IAmNotActorNorTrackable
    : Base<IAmNotActorNorTrackable>
  {};

  struct IAmActor
    : Base<IAmActor>
    , qi::Actor
  {};

  struct IAmTrackable
    : Base<IAmTrackable>
    , qi::Trackable<IAmTrackable>
  {
    ~IAmTrackable()
    {
      this->destroy();
    }
  };

  struct IAmTrackableActor
    : Base<IAmTrackableActor>
    , qi::Actor
    , qi::Trackable<IAmTrackableActor>
  {
    ~IAmTrackableActor()
    {
      this->destroy();
    }
  };

  template<class T>
  using Ptr = boost::shared_ptr<T>;

  template <typename T>
  struct QiLangSafeMemberAsync : ::testing::Test {};
  using SafeMemberAsync_AllowedTypes = ::testing::Types<IAmNotActorNorTrackable,
                                                      IAmActor,
                                                      IAmTrackable,
                                                      IAmTrackableActor>;
  TYPED_TEST_SUITE(QiLangSafeMemberAsync, SafeMemberAsync_AllowedTypes);


  template <typename T>
  struct QiLangSafeMemberAsyncActor : ::testing::Test {};
  using SafeMemberAsync_ActorTypes = ::testing::Types<IAmActor,
                                                      IAmTrackableActor>;
  TYPED_TEST_SUITE(QiLangSafeMemberAsyncActor, SafeMemberAsync_ActorTypes);
}

TYPED_TEST(QiLangSafeMemberAsync, safeMemberAsyncCompilesAndRun)
{
  using qilang::detail::safeMemberAsync;
  using Ptr = Ptr<TypeParam>;

  Ptr ptr = boost::make_shared<TypeParam>();

  auto ft = safeMemberAsync<void, TypeParam>([](Ptr& ptr) mutable {
      return ptr->foo();
    }, ptr);
  EXPECT_EQ(qi::FutureState_FinishedWithValue, ft.wait(waitTimeout));

  auto ftArgs = safeMemberAsync<void, TypeParam>([](Ptr& ptr, int a, int b, int c) mutable {
      return ptr->foo(a, b, c);
    }, ptr, 1, 2, 3);
  EXPECT_EQ(qi::FutureState_FinishedWithValue, ftArgs.wait(waitTimeout));

  auto ftFt = safeMemberAsync<qi::Future<void>, TypeParam>([](Ptr& ptr) mutable {
      return ptr->asyncFoo();
    }, ptr);
  EXPECT_EQ(qi::FutureState_FinishedWithValue, ftFt.unwrap().wait(waitTimeout));

  static const std::vector<std::string> expectedLog{
    "foo()", "foo(int, int, int)", "asyncFoo()",
  };
  EXPECT_EQ(expectedLog, ptr->log);
}

TYPED_TEST(QiLangSafeMemberAsyncActor, safeMemberAsyncConcurrentCalls)
{
  const int countConcurrentCallers = std::thread::hardware_concurrency();
  const int countCallsPerCaller = 3;
  const int totalCallCount = countConcurrentCallers * countCallsPerCaller;

  using std::chrono::duration_cast;
  using std::chrono::milliseconds;
  const qi::MilliSeconds timeout{ duration_cast<milliseconds>(checkDurationPerCall * totalCallCount).count() };

  using qilang::detail::safeMemberAsync;
  using Ptr = Ptr<TypeParam>;

  Ptr ptr = boost::make_shared<TypeParam>();

  auto results = test::qilang::runTaskConcurrently([=]() mutable {
    auto ft = qi::detail::tryUnwrap(safeMemberAsync<qi::Future<void>, TypeParam>([](Ptr& ptr) mutable {
        return ptr->asyncFoo();
      }, ptr));
    EXPECT_EQ(qi::FutureState_FinishedWithValue, ft.wait(timeout));
    }, countCallsPerCaller, countConcurrentCallers);

  for (auto&& result : results)
  {
    ASSERT_EQ(qi::FutureState_FinishedWithValue, result.wait(waitTimeout));
  }

  EXPECT_EQ(static_cast<std::size_t>(totalCallCount), ptr->log.size());
  for (auto&& value : ptr->log)
  {
    EXPECT_EQ("asyncFoo()", value);
  }
}

// TODO: when C++ > 11, uncomment the test below checking if move-only args are allowed.
// Then fix it using C++14's lambda capture expressions to move the arguments in the closure
// implemented in `safeMemberAsync()`.
//
//namespace {
//  struct INeedMoveOnlyArgs
//  {
//    std::unique_ptr<int> value;
//
//    void setValue(std::unique_ptr<int> newValue)
//    {
//      value = std::move(newValue);
//    }
//  };
//}
//
//TEST(QiLangCommon, safeMemberAsyncAllowsMoveOnlyArgs)
//{
//  using qilang::detail::safeMemberAsync;
//  using Ptr = Ptr<INeedMoveOnlyArgs>;
//
//  Ptr ptr = boost::make_shared<INeedMoveOnlyArgs>();
//  const int value = 122334958946;
//  std::unique_ptr<int> ptrValue{ new int(value) };
//
//  auto ftArgs = safeMemberAsync<void, INeedMoveOnlyArgs>([](Ptr& ptr, std::unique_ptr<int> value) mutable {
//    return ptr->setValue(std::move(value));
//  }, ptr, std::move(ptrValue));
//  EXPECT_TRUE(test::finishesWithValue(ftArgs));
//  ASSERT_TRUE(ptr->value);
//  EXPECT_EQ(value, *ptr->value);
//}

