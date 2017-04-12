#include <gtest/gtest.h>
#include <future>
#include <testqilang/somesignals.hpp>
#include "test_qilang.hpp"

using QiLangSignal = QiLangModule;

using namespace testqilang;

TEST_F(QiLangSignal, connectVoid)
{
  auto signalMaster = _testqilang.call<qi::Object<SignalMaster>>("SignalMaster");
  std::promise<void> received;
  signalMaster->voidSignal.connect([&received]() { received.set_value(); });
  QI_EMIT signalMaster->voidSignal();
  ASSERT_EQ(std::future_status::ready, received.get_future().wait_for(usualTimeout));
}

TEST_F(QiLangSignal, connectInt)
{
  auto signalMaster = _testqilang.call<qi::Object<SignalMaster>>("SignalMaster");
  std::promise<int> received;
  const int expected = 42;
  signalMaster->intSignal.connect([&received](int value) { received.set_value(value); });
  QI_EMIT signalMaster->intSignal(expected);
  auto receivedFuture = received.get_future();
  ASSERT_EQ(std::future_status::ready, receivedFuture.wait_for(usualTimeout));
  ASSERT_EQ(expected, receivedFuture.get());
}
