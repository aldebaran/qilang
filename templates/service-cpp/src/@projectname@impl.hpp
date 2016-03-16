#include <src/@projectname@_p.hpp>
#include <qi/session.hpp>

namespace @projectname@
{

class @ProjectName@Impl
{
public:
  // Add your public methods implementation
  // You can add everything you want in public here since only
  // methods comming from the interface will be exported to user API
  @ProjectName@Impl(const qi::SessionPtr& s)
      : _session(s)
  {

  }

  void emitPing(int value) {
    if (pingEnabled.get())
      QI_EMIT ping(value);
  }

  qi::Future<std::vector<int>> workIt(std::string, int) {
    return qi::Future<std::vector<int>>{std::vector<int>{}}; // Immediate or asynchronous tasks are advised
  }

  qi::Signal<int> ping;
  qi::Property<bool> pingEnabled;

private: // This session is useless here, but most people will need it
  qi::SessionPtr _session;
};
} // @projectname@

// Specifies to the type system that @ProjectName@Impl is the implementation of @ProjectName@ API
QI_REGISTER_IMPLEMENTATION_H(@projectname@::@ProjectName@, @projectname@::@ProjectName@Impl);

