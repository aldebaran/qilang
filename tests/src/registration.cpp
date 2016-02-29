#include <qi/anymodule.hpp>
#include "kindamanagerimpl.hpp"
#include "anotherinterfaceimpl.hpp"

REGISTER_KINDAMANAGER(testqilang::KindaManagerImpl)
REGISTER_ANOTHERINTERFACE(testqilang::AnotherInterfaceImpl)

QI_REGISTER_MODULE(
    "testqilang_module",
    [](qi::ModuleBuilder* mb)
{
  mb->advertiseFactory<testqilang::AnotherInterface>("AnotherInterface");
});

