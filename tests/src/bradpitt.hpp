#ifndef TESTQILANG_BRADPITTIMPL_HPP
#define TESTQILANG_BRADPITTIMPL_HPP

#include <src/somemix_p.hpp>

namespace testqilang
{
class BradPittImpl: public qi::Actor
{
public:
  qi::Future<void> act() { return qi::Future<void>{nullptr}; }
};
} // testqilang

QI_REGISTER_IMPLEMENTATION_H(testqilang::BradPitt, testqilang::BradPittImpl)

#endif // TESTQILANG_BRADPITTIMPL_HPP
