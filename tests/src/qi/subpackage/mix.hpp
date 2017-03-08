#include <src/subpackage/mix_p.hpp>
#include "api.hpp"

namespace qi
{
namespace subpackage
{
struct QI_SUBPACKAGE_IMPL_API EntityImpl
{
  Data makeData(Option opt);
  Signal<Data> dataProduced;
  Property<Option> currentOption;
};

QI_SUBPACKAGE_IMPL_API bool operator==(const Data& lhs, const Data& rhs);

} // subpackage
} // qi

QI_REGISTER_IMPLEMENTATION_H(qi::subpackage::Entity, qi::subpackage::EntityImpl)
