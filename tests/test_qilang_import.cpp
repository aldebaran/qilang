#include <testimportpackage/importfromsubpackage.hpp>
#include <testsubpackage/othersubpackage/localimport.hpp>
#include <gtest/gtest.h>

TEST(QiLangImportPackage, makingAStructImportedFromASubpackage)
{
  testimportpackage::LookingAtIt{};
}

TEST(QiLangImportPackage, makingAStructImportedLocallyFromASubpackage)
{
  testsubpackage::othersubpackage::JustAStruct{};
}
