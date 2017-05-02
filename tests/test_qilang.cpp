#include <gmock/gmock.h>
#include <qi/application.hpp>
#include <qi/log.hpp>
#include "test_qilang.hpp"

qi::AnyModule QiLangModule::_testqilang{};

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleMock(&argc, argv);
  qi::log::addFilter("qi*", qi::LogLevel_Debug);
  return RUN_ALL_TESTS();
}
