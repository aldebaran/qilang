/*
** Author(s):
**  - Philippe Daouadi <pdaouadi@aldebaran.com>
**
** Copyright (C) 2015 Aldebaran Robotics
*/

#ifndef QILANG_DOCPARSER_HPP
#define QILANG_DOCPARSER_HPP

#include <string>
#include <map>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>

#include <qilang/api.hpp>

namespace qilang {
  struct Doc {
    typedef std::map<std::string, std::string> Parameters;

    boost::optional<std::string> brief;
    boost::optional<std::string> description;
    Parameters throw_;
    Parameters parameters;
    boost::optional<std::string> return_;
  };

  Doc QILANG_API parseDoc(const std::string& str);
}

#endif
