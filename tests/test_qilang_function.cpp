#include <gtest/gtest.h>
#include <qi/anymodule.hpp>
#include <testqilang/somemix.hpp>
#include <testqilang/somestructs.hpp>

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
