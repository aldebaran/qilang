#ifndef TEST_QILANG_HPP
#define TEST_QILANG_HPP

#include <gtest/gtest.h>
#include <chrono>
#include <gtest/gtest.h>
#include <qi/anymodule.hpp>

static const std::chrono::milliseconds usualTimeout{100};

class QiLangModule: public ::testing::Test
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

#endif // TEST_QILANG_HPP
