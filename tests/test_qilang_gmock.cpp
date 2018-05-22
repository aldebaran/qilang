#include <future>
#include <gmock/gmock.h>
#include <boost/make_shared.hpp>
#include <testqilang/gmock/somemix.hpp>
#include "test_qilang.hpp"

TEST(QiLangGMock, mockExistsAndWorks)
{
  int truth = 1337;
  testqilang::gmock::KindaManagerGMock kindaManagerMock{};
  EXPECT_CALL(kindaManagerMock, findTruth()).WillOnce(testing::Return(qi::Future<int>{truth}));
  ASSERT_EQ(truth, kindaManagerMock.findTruth().value());
}

TEST(QiLangGMock, typeErasedCallToMockedFunction)
{
  int truth = 1337;
  auto kindaManagerMock = boost::make_shared<testqilang::gmock::KindaManagerGMock>();
  EXPECT_CALL(*kindaManagerMock, findTruth()).WillOnce(testing::Return(qi::Future<int>{truth}));
  qi::AnyObject kindaManager{kindaManagerMock};
  ASSERT_EQ(truth, kindaManager.call<int>("findTruth"));
}

TEST(QiLangGMock, typeErasedCallToNiceMockedFunction)
{
  int truth = 1337;
  auto kindaManagerMock = boost::make_shared<testqilang::gmock::KindaManagerNiceGMock>();
  EXPECT_CALL(*kindaManagerMock, findTruth()).WillOnce(testing::Return(qi::Future<int>{truth}));
  qi::AnyObject kindaManager{kindaManagerMock};
  ASSERT_EQ(truth, kindaManager.call<int>("findTruth"));
}

TEST(QiLangGMock, typeErasedCallToStrictMockedFunction)
{
  int truth = 1337;
  auto kindaManagerMock = boost::make_shared<testqilang::gmock::KindaManagerStrictGMock>();
  EXPECT_CALL(*kindaManagerMock, findTruth()).WillOnce(testing::Return(qi::Future<int>{truth}));
  qi::AnyObject kindaManager{kindaManagerMock};
  ASSERT_EQ(truth, kindaManager.call<int>("findTruth"));
}

TEST(QiLangGMock, typeErasedSubscriptionToMockedSignal)
{
  float pi = 3.14f;
  auto kindaManagerMock = boost::make_shared<testqilang::gmock::KindaManagerGMock>();
  qi::AnyObject kindaManager{kindaManagerMock};
  std::promise<float> calledBack;
  kindaManager.connect("test", [&](float v){ calledBack.set_value(v); });
  QI_EMIT kindaManagerMock->test(pi);
  auto future = calledBack.get_future();
  ASSERT_EQ(std::future_status::ready, future.wait_for(usualTimeout));
  ASSERT_EQ(pi, future.get());
}

TEST(QiLangGMock, typeErasedSubscriptionToNiceMockedSignal)
{
  float pi = 3.14f;
  auto kindaManagerMock = boost::make_shared<testqilang::gmock::KindaManagerNiceGMock>();
  qi::AnyObject kindaManager{kindaManagerMock};
  std::promise<float> calledBack;
  kindaManager.connect("test", [&](float v){ calledBack.set_value(v); });
  QI_EMIT kindaManagerMock->test(pi);
  auto future = calledBack.get_future();
  ASSERT_EQ(std::future_status::ready, future.wait_for(usualTimeout));
  ASSERT_EQ(pi, future.get());
}

TEST(QiLangGMock, typeErasedSubscriptionToVoidNiceMockedSignal)
{
  auto kindaManagerMock = boost::make_shared<testqilang::gmock::KindaManagerNiceGMock>();
  qi::AnyObject kindaManager{kindaManagerMock};
  std::promise<void> calledBack;
  kindaManager.connect("nothing", [&](){ calledBack.set_value(); });
  QI_EMIT kindaManagerMock->nothing();
  auto future = calledBack.get_future();
  ASSERT_EQ(std::future_status::ready, future.wait_for(usualTimeout));
}
