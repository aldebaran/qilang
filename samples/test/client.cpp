#include <qi/applicationsession.hpp>
#include <qi/anyobject.hpp>
#include <qi/anymodule.hpp>
#include <test/test.hpp>

void cb(std::string s)
{
  std::cout << "said " << s << std::endl;
}

int main(int argc, char* argv[])
{
  qi::ApplicationSession app(argc, argv);
  app.startSession();
  qi::Object<test::Test> proxy = app.session()->service("test");

  proxy->message.connect(cb);
  proxy.call<void>("saySlowerButBetter", "type erased");
  proxy->saySlowerButBetter("roflol");
  proxy->async().saySlowerButBetter("async roflol");
  proxy->async().saySlowerButBetter("async roflol");
}
