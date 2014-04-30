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
#include "location.hh"
#include "grammar.tab.hpp"


namespace qilang {
  class QILANG_API Parser {
  public:
    Parser(std::istream *stream, const std::string &filename);
    ~Parser();

    NodePtrVector parse();

    void setCurrentPackage(const std::string& pkg);
    const std::string& currentPackage();

  public:
    std::string          package;
    yy::location         loc;
    void*                scanner;  // flex context
    std::vector<NodePtr> root;     // parser output
    std::istream*        in;       // input stream
    std::string          filename;
    yy::parser           parser;
  };

  Location loc(const yy::location& loc);
  std::string getErrorLine(const yy::location& loc);

}

#endif	    /* !PARSER_P_PP_ */
