#include <gtest/gtest.h>
#include <future>
#include <testqilang/someproperties.hpp>
#include "test_qilang.hpp"

using QiLangProperty = QiLangModule;

using namespace testqilang;

TEST_F(QiLangProperty, connectInt)
{
  auto propertyMaster = _testqilang.call<qi::Object<PropertyMaster>>("PropertyMaster");
  std::promise<int> received;
  const int expected = 42;
  propertyMaster->intProperty.connect([&received](int value) { received.set_value(value); });
  propertyMaster->intProperty.set(expected);
  auto receivedFuture = received.get_future();
  ASSERT_EQ(std::future_status::ready, receivedFuture.wait_for(usualTimeout));
  ASSERT_EQ(expected, receivedFuture.get());
  ASSERT_EQ(expected, propertyMaster->intProperty.get().value());
}

TEST_F(QiLangProperty, connectOptional)
{
  auto propertyMaster = _testqilang.call<qi::Object<PropertyMaster>>("PropertyMaster");
  std::promise<boost::optional<float>> received;
  const float expected = 0.5;
  propertyMaster->optionalProperty.connect([&received](boost::optional<float> value) { received.set_value(value); });
  propertyMaster->optionalProperty.set(expected);
  auto receivedFuture = received.get_future();
  ASSERT_EQ(std::future_status::ready, receivedFuture.wait_for(usualTimeout));
  ASSERT_EQ(expected, receivedFuture.get());
  ASSERT_EQ(expected, propertyMaster->optionalProperty.get().value().get());
}
