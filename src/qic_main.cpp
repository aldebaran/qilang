/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <iostream>
#include <qi/log.hpp>
#include <fstream>
#include <qilang/node.hpp>
#include <qilang/parser.hpp>


int main(int argc, char *argv[])
{
  std::cout << "qi!" << std::endl;
  std::istream *in;
  std::ifstream is;

  if (argc == 1)
    in = &std::cin;
  else {
    is.open(argv[1]);
    in = &is;
  }

  qilang::Node *rootnode = qilang::parse(in);

  std::cout << "parsed ast:" << qilang::toSExpr(rootnode) << std::endl;
  std::cout << "shuting down qi." << std::endl;
  return 0;
}
