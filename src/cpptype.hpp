/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Cedric GESTES
*/

#include <string>
#include <vector>
#include <qilang/node.hpp>

namespace qilang {

  std::string typeToCpp(TypeNode* type, bool constref=true);
  std::vector<std::string> splitPkgName(const std::string& name);

}
