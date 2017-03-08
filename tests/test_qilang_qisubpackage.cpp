#include <future>
#include <gtest/gtest.h>
#include <boost/make_shared.hpp>
#include "src/qi/subpackage/mix.hpp"
#include "test_qilang.hpp"

using namespace qi::subpackage;

TEST(QiSubpackage, makeData)
{
  EntityPtr entity = boost::make_shared<EntityImpl>();

  std::promise<Data> emittedData;
  entity->dataProduced.connect([&](Data data) { emittedData.set_value(data); });

  std::promise<Option> emittedOption;
  entity->currentOption.connect([&](Option opt) { emittedOption.set_value(opt); });

  const Option targetOption = Option::First;
  auto returnedData = entity->makeData(targetOption);

  auto gettingData = emittedData.get_future();
  ASSERT_EQ(std::future_status::ready, gettingData.wait_for(usualTimeout));
  EXPECT_EQ(returnedData, gettingData.get());

  auto gettingOption = emittedOption.get_future();
  ASSERT_EQ(std::future_status::ready, gettingOption.wait_for(usualTimeout));
  EXPECT_EQ(targetOption, gettingOption.get());
}
