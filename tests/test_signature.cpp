
#include <iostream>
#include <gtest/gtest.h>
#include <qimessaging/applicationsession.hpp>
#include <qilang/node.hpp>
#include <qilang/formatter.hpp>
#include <qilang/parser.hpp>

TEST(TestSignature, signatureToQiLangSimple) {
    const qi::Signature signature("(bcCwWiIlLfdsm)"); // v,o and X are not tested
    qilang::TypeExprNodePtr node = qilang::signatureToQiLang(signature);

    EXPECT_STREQ("(bool, int8, uint8, int16, uint16, int32, uint32, int64, uint64, float32, float64, str, any)", qilang::format(node).c_str());
}

TEST(TestSignature, signatureToQiLangList) {
  const qi::Signature signature("([b][c][C][w][W][i][I][l][L][f][d][s][m])"); //miss v
  qilang::TypeExprNodePtr node = qilang::signatureToQiLang(signature);

  EXPECT_STREQ("([]bool, []int8, []uint8, []int16, []uint16, []int32, []uint32, []int64, []uint64, []float32, []float64, []str, []any)", qilang::format(node).c_str());
}

TEST(TestSignature, signatureToQiLangTuple) {
  const qi::Signature signature("((cC)(wW)(iI)(lL)(fd)(bsm))"); //miss v
  qilang::TypeExprNodePtr node = qilang::signatureToQiLang(signature);

  EXPECT_STREQ("((int8, uint8), (int16, uint16), (int32, uint32), (int64, uint64), (float32, float64), (bool, str, any))", qilang::format(node).c_str());
}

TEST(TestSignature, signatureToQiLangMap) {
  const qi::Signature signature("({cC}{wW}{iI}{lL}{fd}{bs}{cm})"); //miss v
  qilang::TypeExprNodePtr node = qilang::signatureToQiLang(signature);

  EXPECT_STREQ("([int8]uint8, [int16]uint16, [int32]uint32, [int64]uint64, [float32]float64, [bool]str, [int8]any)", qilang::format(node).c_str());
}

qi::MetaMethod metaMethodeTest(){
  const qi::Signature returnSignature("i");
  const qi::Signature parametersSignature("(bcCiIlLfdsm)");
  qi::MetaMethodParameterVector parameters;
  qi::MetaMethodParameter parameter1("Parameter1", "Parameter1 is a bool");
  qi::MetaMethodParameter parameter2("Parameter2", "Parameter2 is a int8");
  qi::MetaMethodParameter parameter3("parameter3", "parameter3 is a uint8");
  qi::MetaMethodParameter parameter4("parameter4", "parameter4 is a int32");
  qi::MetaMethodParameter parameter5("parameter5", "parameter5 is a uint32");
  qi::MetaMethodParameter parameter6("parameter6", "parameter6 is a int64");
  qi::MetaMethodParameter parameter7("parameter7", "parameter7 is a uint64");
  qi::MetaMethodParameter parameter8("parameter8", "parameter8 is a float32");
  qi::MetaMethodParameter parameter9("parameter9", "parameter9 is a float64");
  qi::MetaMethodParameter parameter10("parameter10", "parameter10 is a str");
  qi::MetaMethodParameter parameter11("parameter11", "parameter11 is a any");

  parameters.push_back(parameter1);
  parameters.push_back(parameter2);
  parameters.push_back(parameter3);
  parameters.push_back(parameter4);
  parameters.push_back(parameter5);
  parameters.push_back(parameter6);
  parameters.push_back(parameter7);
  parameters.push_back(parameter8);
  parameters.push_back(parameter9);
  parameters.push_back(parameter10);
  parameters.push_back(parameter11);

  const qi::MetaMethod metamethode(100, returnSignature, "methodeTest", parametersSignature, "Methode test of metaMethodToQiLang", parameters, "Return int32");
  return metamethode;
}

qi::MetaSignal metaSignalTest(){
  const qi::Signature signature("(c)");
  const qi::MetaSignal signal(100, "signalTest", signature);
  return signal;
}

qi::MetaProperty metaPropertyTest(){
  const qi::Signature signature("(c)");
  const qi::MetaProperty property(100, "propertyTest", signature);
  return property;
}

TEST(TestSignature, metaObjectToQiLang) {
  const std::string pkg = ("packageTest");
  const std::string name = ("ObjectTest");
  //method
  qi::MetaObject::MethodMap methodMap;
  methodMap.insert(std::pair<unsigned int,qi::MetaMethod>(0, metaMethodeTest()));
  //signal
  qi::MetaObject::SignalMap signalMap;
  signalMap.insert(std::pair<unsigned int,qi::MetaSignal>(0, metaSignalTest()));
  //property
  qi::MetaObject::PropertyMap propertyMap;
  propertyMap.insert(std::pair<unsigned int,qi::MetaProperty>(0, metaPropertyTest()));

  const qi::MetaObject object(methodMap, signalMap, propertyMap, "Objet test of metaObjectToQiLang");
  qilang::NodePtr node = qilang::metaObjectToQiLang(name, object);

  EXPECT_STREQ("interface ObjectTest\n  fn methodeTest(bool, int8, uint8, int32, uint32, int64, uint64, float32, float64, str, any) int32\n  out signalTest(int8)\n  prop propertyTest(int8)\nend\n\n", qilang::format(node).c_str());
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
