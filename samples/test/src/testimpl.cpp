#include <test/test.hpp>
#include <qi/anymodule.hpp>
#include <qi/session.hpp>
#include "src/test_p.hpp"

qiLogCategory("test.testimpl");

namespace test
{

class TestImpl : public qi::Actor
{
public:
  // Add your public methods implementation
  // Yan can add everything you want in public here since only
  // methods comming from the interface will be export to user API
  TestImpl(const qi::SessionPtr& s);

  void say(const std::string& mess);
  qi::Future<void> saySlowerButBetter(const std::string& mess);
  int numberSay();

  qi::Signal<int> onSayMessage;
  qi::Property<std::string> message;

private:
  // Add your private methods
  // they will never be publish to user

private:
  // Add your private attribute
  qi::SessionPtr _session;
  int _numberOfSay;
  qi::Atomic<bool> _boole;
};

TestImpl::TestImpl(const qi::SessionPtr& s)
  : _session(s)
  , _numberOfSay(0)
{
}

void TestImpl::say(const std::string& mess)
{
  qiLogInfo() << "Saying " << mess;
}

qi::Future<void> TestImpl::saySlowerButBetter(const std::string& mess)
{
  qiLogInfo() << "Saying " << mess << " slowly";
  assert(!_boole.swap(true));

  qi::sleepFor(qi::Seconds(1));

  ++_numberOfSay;
  message.set(mess);
  onSayMessage(_numberOfSay);

  qiLogInfo() << "Middle say";

  assert(_boole.swap(false));
  return qi::asyncDelay([]{
        qiLogInfo() << "End say";
      }, qi::Seconds(1));
}

int TestImpl::numberSay()
{
  return _numberOfSay;
}

}


QI_REGISTER_IMPLEMENTATION_H(test::Test, test::TestImpl)


// Specifie to the type system that TestImpl is the implementation of the Test interface
REGISTER_TEST(test::TestImpl)

// Create a factory. It will allow the type system to create a AnyObject of your service
// If you are more than one factory put them here
void registerTest(qi::ModuleBuilder* mb)
{
  mb->advertiseFactory<test::Test, qi::SessionPtr>("Test");
}
QI_REGISTER_MODULE("test", &registerTest);
