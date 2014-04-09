/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#ifndef   	VISITOR_HPP_
# define   	VISITOR_HPP_

#include <qilang/api.hpp>
#include <qilang/node.hpp>
#include <sstream>

namespace qilang {

  QILANG_API std::string genCppObjectInterface(const NodePtr& node);
  QILANG_API std::string genCppObjectInterface(const NodePtrVector& nodes);

  QILANG_API std::string formatAST(const NodePtrVector& node);
  QILANG_API std::string format(const NodePtrVector& node);

  QILANG_API std::string formatAST(const NodePtr& node);
  QILANG_API std::string format(const NodePtr& node);

}

#endif	    /* !VISITOR_PP_ */
