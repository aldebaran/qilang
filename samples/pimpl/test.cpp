/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <iostream>
#include <pimpl/bar.hpp>


int main(int argc, char *argv[])
{
  pimpl::Bar b = pimpl::newBar();

  b->foo("42");
  return 0;
}
