/*
** qiLang generated file. DO NOT EDIT
*/
#include <qitype/objecttypebuilder.hpp>
#include <string>
#include <vector>
#include <map>
#include <pimpl/bar.hpp>

namespace pimpl {

static int initType0() {
  qi::ObjectTypeBuilder< ::pimpl::BarInterface > builder;
  builder.advertiseMethod("foo", &::pimpl::BarInterface::foo);
  return 42;
}
static int myinittype0 = initType0();

}
