#include <qi/anymodule.hpp>
#include <testqilang/somemix.hpp>
#include <testqilang/somestructs.hpp>
#include <testqilang/time.hpp>
#include "kindamanagerimpl.hpp"
#include "anotherinterfaceimpl.hpp"
#include "bradpitt.hpp"
#include "buffy.hpp"
#include "ouroboros.hpp"
#include "pingpong.hpp"
#include "propertymaster.hpp"
#include "signalmaster.hpp"
#include "time.hpp"
#include "truthseeker.hpp"

REGISTER_KINDAMANAGER(testqilang::KindaManagerImpl)
REGISTER_ANOTHERINTERFACE(testqilang::AnotherInterfaceImpl)
REGISTER_BRADPITT(testqilang::BradPittImpl)
REGISTER_BUFFY(testqilang::BuffyImpl)
REGISTER_TIMELORD(testqilang::TimeLordImpl)
REGISTER_OUROBOROS(testqilang::OuroborosImpl)
REGISTER_PING(testqilang::PingImpl)
REGISTER_PONG(testqilang::PongImpl)
REGISTER_PROPERTYMASTER(testqilang::PropertyMasterImpl)
REGISTER_SIGNALMASTER(testqilang::SignalMasterImpl)
REGISTER_TRUTHSEEKER(testqilang::TruthSeekerImpl)
REGISTER_FOO(testqilang::FooImpl)

testqilang::Error makeError(int code, std::string message)
{
  testqilang::Error error;
  error.code = code;
  error.message = message;
  return error;
}

testqilang::Score makeScore(int score, std::string name)
{
  testqilang::Score entry;
  entry.score = score;
  entry.name = name;
  return entry;
}

int sumScores(std::vector<testqilang::Score> scores)
{
  int total = 0;
  for (const auto& score: scores)
    total += score.score;
  return total;
}

void registerTestQiLang(qi::ModuleBuilder* mb)
{
  mb->advertiseFactory<testqilang::KindaManager>("KindaManager");
  mb->advertiseFactory<testqilang::AnotherInterface>("AnotherInterface");
  mb->advertiseFactory<testqilang::BradPitt>("BradPitt");
  mb->advertiseFactory<testqilang::Buffy>("Buffy");
  mb->advertiseFactory<testqilang::SignalMaster>("SignalMaster");
  mb->advertiseFactory<testqilang::PropertyMaster>("PropertyMaster");
  mb->advertiseFactory<testqilang::TimeLord>("TimeLord");
  mb->advertiseFactory<testqilang::Foo>("Foo");
  mb->advertiseMethod("Error", makeError);
  mb->advertiseMethod("Score", makeScore);
  mb->advertiseMethod("sumScores", sumScores);
}

QI_REGISTER_MODULE("testqilang_module", registerTestQiLang)

