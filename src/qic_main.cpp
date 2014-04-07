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

  qilang::NodePtr rootnode = qilang::parse(in);

  std::cout << "AST:" << std::endl;
  std::cout << qilang::formatAST(rootnode);

  std::cout << std::endl;
  std::cout << "QILANG:" << std::endl;
  std::cout << qilang::format(rootnode);
  return 0;
}
