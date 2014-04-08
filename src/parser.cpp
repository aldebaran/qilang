/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <qi/os.hpp>
#include <qilang/parser.hpp>
#include <qilang/node.hpp>
#include "parser_p.hpp"
#include <iostream>
#include <fstream>
#include "grammar.tab.hpp"

int  qilang_lex_init(void**);
int  qilang_lex_destroy(void*);
void qilang_set_extra(qilang::Parser*, void *);
struct yyscan_t;
void qilang_set_debug(int debug_flag, void* yyscanner);

namespace qilang {

  Parser::Parser(std::istream *stream, const std::string &filename)
    : in(stream)
    , filename(filename)
  {
    qilang_lex_init(&scanner);
    qilang_set_extra(this, scanner);
  }

  Parser::~Parser()
  {
    qilang_lex_destroy(scanner);
  }

  NodePtrVector Parser::parse() {
    yy::parser parser(this);

    loc.initialize(&filename);
    std::string pdebug = qi::os::getenv("QILANG_PARSER_DEBUG");
    if (!pdebug.empty() && pdebug != "0") {
      parser.set_debug_level(1);
    }
    else {
      parser.set_debug_level(0);
    }
    std::string ldebug = qi::os::getenv("QILANG_LEXER_DEBUG");
    if (!ldebug.empty() && ldebug != "0") {
      qilang_set_debug(1, scanner);
    }
    else {
      qilang_set_debug(0, scanner);
    }
    //try {
    parser.parse();
    //} catch (const ParserError& pe) {
    //  pe.setFileName(filename);
    //  throw;
    //}
    return root;
  }

  NodePtrVector parse(const std::string &filename) {
    std::ifstream is;
    is.open(filename.c_str());
    Parser p(&is, filename);
    return p.parse();
  }

  NodePtrVector parse(std::istream *stream, const std::string& filename) {
    Parser p(stream, filename);
    return p.parse();
  }

}
