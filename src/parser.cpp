/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <qilang/parser.hpp>
#include <qilang/node.hpp>
#include "parser_p.hpp"
#include <iostream>
#include "grammar.tab.hpp"

int  qilang_lex_init(void**);
int  qilang_lex_destroy(void*);
void qilang_set_extra(qilang::Parser*, void *);

namespace qilang {

  Parser::Parser(std::istream *stream)
    : in(stream)
  {
    qilang_lex_init(&scanner);
    qilang_set_extra(this, scanner);
  }

  Parser::~Parser()
  {
    qilang_lex_destroy(scanner);
  }

  NodePtr Parser::parse() {
    yy::parser parser(this);
    parser.set_debug_level(42);
    parser.parse();
    return root;
  }

  NodePtr parse(std::istream *stream) {
    Parser p(stream);
    return p.parse();
  }

}
