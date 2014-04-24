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
    , parser(this)
  {
    qilang_lex_init(&scanner);
    qilang_set_extra(this, scanner);
  }

  Parser::~Parser()
  {
    qilang_lex_destroy(scanner);
  }

  void Parser::setCurrentPackage(const std::string& pkg) {
    package = pkg;
  }

  const std::string& Parser::currentPackage() {
    if (package.empty())
      throw std::runtime_error("no package specified");
    return package;
  }

  NodePtrVector Parser::parse() {

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
    parser.parse();
    return root;
  }

  std::string getErrorLine(const yy::location& loc) {
    std::ifstream is;
    std::string   ret;
    if (loc.begin.filename == 0)
      return "";
    is.open(loc.begin.filename->c_str());

    std::string ln;
    unsigned int lico = loc.begin.line;
    for (int i = 0; i < lico; ++i)
      getline(is, ln);
    ret = "in:";
    ret += ln + "\n";
    ret += "   ";
    unsigned int cbeg = loc.begin.column;
    unsigned int cend = loc.end.column;
    int count = cend - cbeg;
    int space = cbeg;
    space = space < 0 ? 0 : space;
    count = count < 1 ? 1 : count;
    for (int i = 0; i < space; ++i)
      ret += " ";
    for (int i = 0; i < count; ++i)
      ret += "^";
    ret += "\n";
    return ret;
  }

  NodePtrVector parse(const std::string &filename) {
    std::ifstream is;
    is.open(filename.c_str());
    if (!is.is_open())
      throw std::runtime_error("Can't open file: " + filename);

    Parser p(&is, filename);
    return p.parse();
  }

  NodePtrVector parse(std::istream *stream, const std::string& filename) {
    Parser p(stream, filename);
    return p.parse();
  }

}
