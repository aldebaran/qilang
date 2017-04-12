#ifndef TESTQILANG_PROPERTYMASTERIMPL_HPP
#define TESTQILANG_PROPERTYMASTERIMPL_HPP

#include <src/someproperties_p.hpp>

namespace testqilang
{
struct PropertyMasterImpl
{
  qi::Property<int> intProperty;
};
} // testqilang

QI_REGISTER_IMPLEMENTATION_H(testqilang::PropertyMaster, testqilang::PropertyMasterImpl)

#endif // TESTQILANG_PROPERTYMASTERIMPL_HPP
