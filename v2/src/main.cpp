/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <iostream>
#include <qilang2/node.hpp>



int main(int argc, char *argv[])
{
  qilang2::ExprNode n = qilang2::newVarExprNode("plouf", qilang2::Location());

  qilang2::Node n2 = n;

  std::cout << "n  name:" << n->type() << std::endl;
  std::cout << "n2 name:" << n2->type() << std::endl;
  return 0;
}
