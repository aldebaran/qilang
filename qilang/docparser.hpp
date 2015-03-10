/*
** Author(s):
**  - Philippe Daouadi <pdaouadi@aldebaran.com>
**
** Copyright (C) 2015 Aldebaran Robotics
*/

#ifndef QILANG_DOCPARSER_HPP
#define QILANG_DOCPARSER_HPP

#include <string>
#include <vector>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>

#include <qilang/api.hpp>

BOOST_FUSION_DEFINE_STRUCT(
    (qilang),
    ParamDecl,
    (std::string, name)
    (std::string, description)
)

BOOST_FUSION_DEFINE_STRUCT(
    (qilang),
    ReturnDecl,
    (std::string, description)
)

BOOST_FUSION_DEFINE_STRUCT(
    (qilang),
    ThrowDecl,
    (std::string, description)
)

namespace qilang {
  typedef boost::variant<ParamDecl, ReturnDecl, ThrowDecl> Decl;
  typedef std::vector<Decl> DeclVec;
}

BOOST_FUSION_DEFINE_STRUCT(
    (qilang),
    DocInternal,
    (std::vector<std::string>, description)
    (qilang::DeclVec, declarations)
)

namespace qilang {
  struct Doc {
    boost::optional<std::string> brief;
    boost::optional<std::string> description;
    DeclVec declarations;
  };

  Doc QILANG_API parseDoc(const std::string& str);
}

#endif
