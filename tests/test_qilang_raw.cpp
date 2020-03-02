#include <gtest/gtest.h>
#include <testqilang/buffy.hpp>
#include "test_qilang.hpp"

class QiLangRaw: public QiLangModule
{
protected:
  void SetUp() override
  {
    QiLangModule::SetUp();
    _buffy = _testqilang.call<testqilang::BuffyPtr>("Buffy");
  }

  void TearDown() override
  {
    _buffy.reset();
    QiLangModule::TearDown();
  }

  testqilang::BuffyPtr _buffy;
};


TEST_F(QiLangRaw, identityMethod)
{
  qi::Buffer buffer;
  ASSERT_EQ(buffer, _buffy->bufferIdentity(buffer));
}


TEST_F(QiLangRaw, usingStruct)
{
  static const std::string vampireData = "om nama narayana";
  testqilang::Vampire vampire;
  vampire.data.write(vampireData.data(), vampireData.size());
  vampire.annotation = "Spike";

  auto deadVampire = _buffy->fightAgainst(vampire);
  ASSERT_EQ(vampire.data, deadVampire.data);
  ASSERT_EQ(vampire.annotation, deadVampire.annotation);
}
