#include <gtest/gtest.h>
#include <qi/anymodule.hpp>
#include <testqilang/somemix.hpp>
#include <testqilang/somestructs.hpp>

class QiLangTypeRegistration: public ::testing::Test
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

qi::AnyModule QiLangTypeRegistration::_testqilang{};

using namespace testqilang;

TEST_F(QiLangTypeRegistration, MakeObjectFromModule)
{
  _testqilang.call<qi::Object<AnotherInterface>>("AnotherInterface");
}

TEST_F(QiLangTypeRegistration, MakeAnyObjectFromModule)
{
  _testqilang.call<qi::AnyObject>("AnotherInterface");
}

TEST_F(QiLangTypeRegistration, MakeChildObject)
{
  auto obj = _testqilang.call<qi::Object<AnotherInterface>>("AnotherInterface");
  obj->makeSomething();
}

TEST_F(QiLangTypeRegistration, MakeChildAnyObject)
{
  auto obj = _testqilang.call<qi::Object<AnotherInterface>>("AnotherInterface");
  qi::AnyObject{obj->makeSomething()};
}

TEST_F(QiLangTypeRegistration, CallChildObject)
{
  auto obj = _testqilang.call<qi::Object<AnotherInterface>>("AnotherInterface");
  auto childObj = obj->makeSomething();
  ASSERT_EQ(42, childObj->findTruth());
}

TEST_F(QiLangTypeRegistration, CallChildObjectWithTime)
{
  auto obj = _testqilang.call<qi::Object<AnotherInterface>>("AnotherInterface");
  auto childObj = obj->makeSomething();
  childObj->whatsTheTime();
}

TEST_F(QiLangTypeRegistration, EmitWithTime)
{
  auto obj = _testqilang.call<qi::Object<AnotherInterface>>("AnotherInterface");
  obj->blop(qi::Clock::now());
}

TEST_F(QiLangTypeRegistration, MakeStruct)
{
  auto code = 42;
  std::string message = "oh miiiiince";
  auto error = _testqilang.call<Error>("Error", code, message);
  EXPECT_EQ(code, error.code);
  EXPECT_EQ(message, error.message);
}

TEST_F(QiLangTypeRegistration, StructOnlyFile)
{
  auto score = 666;
  std::string name = "da beast";
  auto scoreStruct = _testqilang.call<Score>("Score", score, name);
  EXPECT_EQ(score, scoreStruct.score);
  EXPECT_EQ(name, scoreStruct.name);
}
