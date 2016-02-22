#include <gtest/gtest.h>
#include <qi/anymodule.hpp>
#include <src/testinterface_p.hpp>
#include <testqilang/testinterface.hpp>

namespace testqilang
{
class KindaManagerImpl
{
public:
  qi::Future<int> setDefaultDevice(const std::string& a, const std::string& b)
  {
    return qi::Future<int>(42);
  }

  qi::Signal<float> test;
  qi::Property<std::vector<float>> current;
};
} // testqilang

REGISTER_KINDAMANAGER(testqilang::KindaManagerImpl)

using namespace testqilang;

TEST(QiLangTypes, ConstructInterfaceFromImplementation)
{
  auto thatInstance = boost::make_shared<KindaManagerImpl>();
  qi::Object<KindaManager>{thatInstance};
}

TEST(QiLangTypes, CallMethodOnObject)
{
  auto thatInstance = boost::make_shared<KindaManagerImpl>();
  qi::Object<KindaManager> asObject{thatInstance};
  ASSERT_EQ(42, asObject->setDefaultDevice("trou", "poil"));
}

TEST(QiLangTypes, ConstructAnyObjectFromImplementation)
{
  auto thatInstance = boost::make_shared<KindaManagerImpl>();
  qi::AnyObject asObject{thatInstance};
}

TEST(QiLangTypes, CallMethodOnAnyObject)
{
  auto thatInstance = boost::make_shared<KindaManagerImpl>();
  qi::AnyObject asObject{thatInstance};
  ASSERT_EQ(42, asObject.call<int>("setDefaultDevice", "trou", "poil"));
}
