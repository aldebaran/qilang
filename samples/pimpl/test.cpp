/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <iostream>
#include <pimpl/bar.hpp>
#include <qimessaging/session.hpp>


int main(int argc, char *argv[])
{
  pimpl::Bar b = pimpl::newBar();

  qi::AnyObject ao = b;

  b->foo("42");

  ao.call<void>("foo", "33");
  qi::Object<pimpl::BarInterface> obj = ao;

  obj->foo("41");



  qi::SessionPtr s = qi::makeSession();
  qi::SessionPtr s2 = qi::makeSession();

  s->listenStandalone("tcp://0.0.0.0:5555");

  s2->connect(s->endpoints()[0]);

  s->registerService("bim", b);

  pimpl::Bar b2 = s2->service("bim");
  b2->foo("yatta");

  return 0;
}
