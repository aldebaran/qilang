/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#ifndef  PARSER_P_HPP_
# define PARSER_P_HPP_

#include <string>

namespace qilang {
  class QILANG_API Parser {
  public:
    Parser(std::istream *stream);
    ~Parser();

    qilang::NodePtr parse();

  public:
    std::string   filename;
    void*         scanner;  // flex context
    NodePtr       root;     // parser output
    std::istream* in;       // input stream
  };

}

#endif	    /* !PARSER_P_PP_ */
