#include <qi/anymodule.hpp>
#include "@projectname@impl.hpp"

REGISTER_@PROJECTNAME@(@projectname@::@ProjectName@Impl)

// Advertise a factory.
// It will allow the type system to create a qi::Object of your service.
// You can add as many factories as you like here, and even functions.
void register@ProjectName@(qi::ModuleBuilder* mb)
{
  mb->advertiseFactory<@projectname@::@ProjectName@, const qi::SessionPtr&>("@ProjectName@");
}
QI_REGISTER_MODULE("@projectname@_module", &register@ProjectName@);

