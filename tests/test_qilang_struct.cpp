#include <gtest/gtest.h>
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
