/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#ifndef QILANG_PARSER_HPP
#define QILANG_PARSER_HPP

#include <qilang/api.hpp>
#include <iosfwd>
#include <qilang/node.hpp>

namespace qilang {

  QILANG_API NodePtrVector parse(std::istream *stream);

}

#endif // QIPARSER_HPP
