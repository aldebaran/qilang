#include <gtest/gtest.h>
#include <testqilang/time.hpp>
#include "test_qilang.hpp"

class QiLangTime: public QiLangModule
{
protected:
  static void SetUpTestCase()
  {
    QiLangModule::SetUpTestCase();
    _timeLord = _testqilang.call<testqilang::TimeLordPtr>("TimeLord");
  }

  static void TearDownTestCase()
  {
    _timeLord.reset();
    QiLangModule::TearDownTestCase();
  }

  static testqilang::TimeLordPtr _timeLord;
};

testqilang::TimeLordPtr QiLangTime::_timeLord{};

TEST_F(QiLangTime, nsec)
{
  qi::NanoSeconds t{42};
  ASSERT_EQ(t, _timeLord->nsecIdentity(t));
}

TEST_F(QiLangTime, usec)
{
  qi::MicroSeconds t{42};
  ASSERT_EQ(t, _timeLord->usecIdentity(t));
}

TEST_F(QiLangTime, msec)
{
  qi::MilliSeconds t{42};
  ASSERT_EQ(t, _timeLord->msecIdentity(t));
}

TEST_F(QiLangTime, sec)
{
  qi::Seconds t{42};
  ASSERT_EQ(t, _timeLord->secIdentity(t));
}

TEST_F(QiLangTime, min)
{
  qi::Minutes t{42};
  ASSERT_EQ(t, _timeLord->minIdentity(t));
}

TEST_F(QiLangTime, hour)
{
  qi::Hours t{42};
  ASSERT_EQ(t, _timeLord->hourIdentity(t));
}
