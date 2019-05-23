#include <gmock/gmock.h>
#include <qi/application.hpp>
#include <qi/log.hpp>
#include <testsession/testsession.hpp>
#include "test_qilang.hpp"

qi::AnyModule QiLangModule::_testqilang{};

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleMock(&argc, argv);
  qi::log::addFilter("qi*", qi::LogLevel_Debug);

  // Enough here to test struct type conversions
  TestMode::forceTestMode(TestMode::Mode_SD);

  // In real use case, struct name in new API version is the same as in older versions
  qi::os::setenv("QI_IGNORE_STRUCT_NAME", "1");

  return RUN_ALL_TESTS();
}
