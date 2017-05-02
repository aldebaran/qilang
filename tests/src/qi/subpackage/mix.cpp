#include "mix.hpp"

namespace qi
{
namespace subpackage
{
Data EntityImpl::makeData(Option opt)
{
  currentOption.set(opt);
  Data data;
  switch (opt)
  {
  case Option::First:
    data.value = 1;
    data.description = "one";
      break;
  case Option::Second:
    data.value = 2;
    data.description = "two";
  }

  QI_EMIT dataProduced(data);
  return data;
}

bool operator==(const Data& lhs, const Data& rhs)
{
  return lhs.value == rhs.value && lhs.description == rhs.description;
}
} // subpackage
} // qi

REGISTER_ENTITY(qi::subpackage::EntityImpl);
