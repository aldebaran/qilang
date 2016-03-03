#include <qi/anymodule.hpp>
#include "kindamanagerimpl.hpp"
#include "anotherinterfaceimpl.hpp"
#include "ouroboros.hpp"

REGISTER_KINDAMANAGER(testqilang::KindaManagerImpl)
REGISTER_ANOTHERINTERFACE(testqilang::AnotherInterfaceImpl)
REGISTER_OUROBOROS(testqilang::OuroborosImpl)

QI_REGISTER_MODULE(
    "testqilang_module",
    [](qi::ModuleBuilder* mb)
{
  mb->advertiseFactory<testqilang::AnotherInterface>("AnotherInterface");
});

