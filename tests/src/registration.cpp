#include <qi/anymodule.hpp>
#include "kindamanagerimpl.hpp"
#include "anotherinterfaceimpl.hpp"
#include "ouroboros.hpp"
#include "pingpong.hpp"

REGISTER_KINDAMANAGER(testqilang::KindaManagerImpl)
REGISTER_ANOTHERINTERFACE(testqilang::AnotherInterfaceImpl)
REGISTER_OUROBOROS(testqilang::OuroborosImpl)
REGISTER_PING(testqilang::PingImpl)
REGISTER_PONG(testqilang::PongImpl)

testqilang::Error makeError(int code, std::string message)
{
  testqilang::Error error;
  error.code = code;
  error.message = message;
  return error;
}

QI_REGISTER_MODULE(
    "testqilang_module",
    [](qi::ModuleBuilder* mb)
{
  mb->advertiseFactory<testqilang::AnotherInterface>("AnotherInterface");
  mb->advertiseMethod("Error", makeError);
})

