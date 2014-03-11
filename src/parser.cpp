/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <qilang/parser.hpp>
#include "parser_p.hpp"
#include <iostream>

int qilang_lex_init(void**);
int qilang_lex_destroy(void*);

void qilang_set_extra(qilang::Parser*, void *);
int  qilang_parse(qilang::Parser *pc);

namespace qilang {

  Parser::Parser(std::istream *stream)
    : root(0)
    , in(stream)
  {
    qilang_lex_init(&scanner);
    qilang_set_extra(this, scanner);
  }

  Parser::~Parser()
  {
    qilang_lex_destroy(scanner);
  }

  Node* Parser::parse() {
    qilang_parse(this);
    return root;
  }

  Node* parse(std::istream *stream) {
    Parser p(stream);
    return p.parse();
  }

}
