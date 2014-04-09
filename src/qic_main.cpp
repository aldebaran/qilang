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
#include <qilang/formatter.hpp>

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

  qilang::NodePtrVector rootnode;
  try {
    rootnode = qilang::parse(in, argv[1]);
  } catch(const std::exception& e) {
    std::cout << e.what();
    exit(1);
  }
  std::cout << "AST:" << std::endl;
  std::cout << qilang::formatAST(rootnode);

  std::cout << std::endl;
  std::cout << "QILANG:" << std::endl;
  std::cout << qilang::format(rootnode);


  std::cout << std::endl;
  std::cout << "HPP:" << std::endl;
  std::cout << qilang::genCppObjectInterface(rootnode);

  return 0;
}
