#include <qi/applicationsession.hpp>
#include <qi/anyobject.hpp>
#include <qi/anymodule.hpp>
#include <test/test.hpp>

qiLogCategory("test.server");

void cb(std::string s)
{
  qiLogInfo() << "callback: said " << s;
}

int main(int argc, char* argv[])
{
  qi::ApplicationSession app(argc, argv);
  app.startSession();
  qi::AnyObject aobj = qi::import("test").call<qi::AnyObject>("Test", app.session());
  qi::Object<test::Test> proxy = aobj;
  aobj = qi::AnyObject();

  proxy->message.connect(cb);
  proxy->saySlowerButBetter("test before");
  proxy->async().saySlowerButBetter("test async");
  proxy->async().saySlowerButBetter("test async");

  app.session()->registerService("test", proxy);

  app.run();
}
