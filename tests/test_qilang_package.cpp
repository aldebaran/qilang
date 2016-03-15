#include <gtest/gtest.h>
#include <testsubpackage/othersubpackage/othersubpackage.hpp>
#include <testimportpackage/testimportpackage.hpp>
#include <src/othersubpackage/othersubpackage_p.hpp>

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

class LetMeSeeImpl
{
public:
  bool ping() { return true; }
};

QI_REGISTER_IMPLEMENTATION_H(testsubpackage::othersubpackage::LetMeSee, LetMeSeeImpl)
REGISTER_LETMESEE(LetMeSeeImpl)

TEST(TestSubPackage, objectInSubPackage)
{
  qi::Object<testsubpackage::othersubpackage::LetMeSee> o { boost::make_shared<LetMeSeeImpl>() };
  ASSERT_TRUE(o->ping());
}
