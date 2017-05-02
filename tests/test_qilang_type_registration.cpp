#include <gtest/gtest.h>
#include <testqilang/somemix.hpp>
#include <testqilang/somestructs.hpp>
#include "test_qilang.hpp"

using QiLangTypeRegistration = QiLangModule;

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

TEST_F(QiLangTypeRegistration, CallChildObjectWithEnum)
{
  auto obj = _testqilang.call<qi::Object<AnotherInterface>>("AnotherInterface");
  auto childObj = obj->makeSomething();
  ASSERT_EQ(Option::First, childObj->setOption(Option::First));
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

TEST_F(QiLangTypeRegistration, sumScores)
{
  std::vector<testqilang::Score> scores;
  int sum = 0;
  for (int i = 0; i < 12; ++i)
  {
    testqilang::Score score;
    score.score = i;
    std::stringstream ss;
    ss << i;
    score.name = ss.str();
    scores.push_back(score);
    sum += i;
  };
  EXPECT_EQ(sum, _testqilang.call<int>("sumScores", scores));
}

TEST_F(QiLangTypeRegistration, StructDefaultValue)
{
  const double epsilon = 0.0001;
  testqilang::Point point;
  ASSERT_NEAR(3.14159, point.x, epsilon);
  ASSERT_NEAR(1.618, point.y, epsilon);
}
