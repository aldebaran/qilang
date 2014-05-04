/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#ifndef  QILANG_PARSER_P_HPP_
# define QILANG_PARSER_P_HPP_

#include <string>
#include <qilang/node.hpp>
#include <qilang/parser.hpp>
#include "location.hh"
#include "grammar.tab.hpp"


namespace qilang {

  struct ParserContext {
  };

  class ParseException {
  public:
    ParseException(const Location& loc, const std::string& what)
      : _loc(loc)
      , _what(what)
    {}
    const Location&    loc() const  { return _loc; }
    const std::string& what() const { return _what; }

  protected:
    Location    _loc;
    std::string _what;
  };

  class QILANG_API Parser: public ParserContext {
  public:
    Parser(const FileReaderPtr &file);
    ~Parser();

    void parse();

    ParseResultPtr   result();

    //parser context
    FileReaderPtr        file;
    ParseResultPtr       _result;
    bool                 _parsed;

    std::string          package;
    yy::location         loc;

    // flex / bison struct
    void*                scanner;  // flex context
    yy::parser           parser;

  };

  Location makeLocation(const yy::location& loc);
  std::string getErrorLine(const std::string& filename, const Location& loc);

}

#endif	    /* !PARSER_P_PP_ */
