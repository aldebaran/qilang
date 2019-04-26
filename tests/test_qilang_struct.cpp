#include <gtest/gtest.h>
#include <testsession/testsessionpair.hpp>
#include <testqilang/somestructs.hpp>

TEST(Struct, defaultConstruction)
{
  testqilang::Score score;
}

TEST(Struct, initializerListConstruction)
{
  testqilang::Score score{42, "chapeau"};
  ASSERT_EQ(42, score.score);
  ASSERT_EQ("chapeau", score.name);
}

TEST(Struct, copyable)
{
  testqilang::Score scoreToMove{42, "chapeau"};
  testqilang::Score score = scoreToMove;
  ASSERT_EQ(42, score.score);
  ASSERT_EQ("chapeau", score.name);
}

TEST(Struct, movable)
{
  testqilang::Score scoreToMove{42, "chapeau"};
  testqilang::Score score = std::move(scoreToMove);
  ASSERT_EQ(42, score.score);
  ASSERT_EQ("chapeau", score.name);
}

// Check generated struct fields assignment
TEST(Struct, optionalFieldsAssignable)
{
  testqilang::Config2 config2;
  config2.n = 42;
  ASSERT_EQ(boost::optional<float>{}, config2.maxSpeed);
  ASSERT_EQ(42, config2.n);
}

// Check local struct conversion when missing optional fields
TEST(Struct, missingOptionalFieldsConversion)
{
  testqilang::Config2 config2;
  config2.n = 42;
  testqilang::Config3 config3;
  config3.n = 42;
  auto config3From2 = qi::AnyReference::from(config2).to(config3);
  ASSERT_EQ(config3From2.maxSpeed, config3.maxSpeed);
  ASSERT_EQ(config3From2.n, config3.n);
  ASSERT_EQ(config3.name, config3From2.name);
}

// Check local struct conversion when missing both optional and mandatory fields
TEST(Struct, missingMandatoryAndOptionalFieldsConversion)
{
  testqilang::Config2 config2;
  config2.n = 42;
  testqilang::Config3APIBreak config3;
  config3.n = 42;
  ASSERT_THROW(qi::AnyReference::from(config2).to(config3), std::runtime_error);
}


class StructProvider {
public:
  testqilang::Config1 config() {
    testqilang::Config1 c;
    c.maxSpeed = 1.3;
    return c;
  }
};
QI_REGISTER_OBJECT(StructProvider,config);

// Check conversion of struct received via RPC when missing optional fields
TEST(Struct, remoteCallArgumentConversion)
{
  TestSessionPair p;
  p.server()->registerService("StructProvider", boost::make_shared<StructProvider>());
  auto structProvider = p.client()->service("StructProvider");
  auto config3From1 = structProvider.value().call<testqilang::Config3>("config");

  testqilang::Config3 config3;
  config3.maxSpeed = StructProvider().config().maxSpeed;
  ASSERT_EQ(config3From1.maxSpeed, config3.maxSpeed);
  ASSERT_EQ(config3From1.n, config3.n);
  ASSERT_EQ(config3From1.name, config3.name);
}
