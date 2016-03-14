#include <gtest/gtest.h>
#include <testsubpackage/othersubpackage/othersubpackage.hpp>
#include <testimportpackage/testimportpackage.hpp>

// Just check that the subpackages import compile, generate headers at the
// right places, and that the imported types can be used.
TEST(TestSubPackage, includeSubPackage)
{
  testsubpackage::othersubpackage::CustomStruct myCustomStruct;
  testqilang::Score score;
  score.score = 5555;
  score.name = "Veridis Quo";
  myCustomStruct.toto = score;
}

TEST(TestSubPackage, importSubPackage)
{
  testimportpackage::ImportedStuffWrapper wrapper;
  wrapper.v.toto.score = 42;
}
