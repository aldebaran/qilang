/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#ifndef  PARSER_P_HPP_
# define PARSER_P_HPP_

#include <string>
#include <qilang/node.hpp>

namespace qilang {
  class QILANG_API Parser {
  public:
    Parser(std::istream *stream);
    ~Parser();

    NodePtrVector parse();

  public:
    std::string          filename;
    void*                scanner;  // flex context
    std::vector<NodePtr> root;     // parser output
    std::istream*        in;       // input stream
  };
}

#endif	    /* !PARSER_P_PP_ */
