#ifndef TESTQILANG_PINGPONG_HPP
#define TESTQILANG_PINGPONG_HPP

#include <src/testinterface_p.hpp>

namespace testqilang
{
class PongImpl;

class PingImpl
{
public:
  qi::Object<Pong> pong() {
    return boost::make_shared<PongImpl>();
  }
};

class PongImpl
{
public:
  qi::Object<Ping> ping() {
    return boost::make_shared<PingImpl>();
  }
};
} // testqilang

QI_REGISTER_IMPLEMENTATION_H(testqilang::Ping, testqilang::PingImpl)
QI_REGISTER_IMPLEMENTATION_H(testqilang::Pong, testqilang::PongImpl)

#endif // TESTQILANG_PINGPONG_HPP
