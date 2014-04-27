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

  Parser::Parser(const FileReaderPtr &file)
    : file(file)
    , _parsed(false)
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

  void Parser::parse() {
    if (_parsed)
      return;
    _parsed = true;
    loc.initialize();
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
    try {
      parser.parse();
    } catch (const ParseException& pe) {
      _result.ast.clear();
      _result.messages.push_back(Message(MessageType_Error, pe.what(), file->filename(), pe.loc()));
    }
  }

  void ParseResult::printMessage(std::ostream &out) {

    for (unsigned i = 0; i < messages.size(); ++i) {
      Message& msg = messages.at(i);

      out << msg.filename() << ":";
      out << msg.loc().beg_line << ":" << msg.loc().beg_column << ": ";

      switch (msg.type()) {
        case MessageType_Error:
          out << "error: ";
          break;
        case MessageType_Warning:
          out << "warning: ";
          break;
        case MessageType_Info:
          out << "info: ";
          break;
        default:
          break;
      }

      out << msg.what() << std::endl;
      out << qilang::getErrorLine(msg.filename(), msg.loc());
    }
  }

  ParseResult Parser::result() {
    parse();
    return _result;
  }

  Location makeLocation(const yy::location& loc) {
    return Location(loc.begin.line, loc.begin.column, loc.end.line, loc.end.column);
  }

  std::string getErrorLine(const std::string& filename, const Location& loc) {
    std::ifstream is;
    std::string   ret;

    is.open(filename.c_str());
    if (!is.is_open())
      return std::string();

    std::string ln;
    unsigned int lico = loc.beg_line;
    for (int i = 0; i < lico; ++i) {
      if (!is.good())
        return std::string();
      getline(is, ln);
    }
    //ret = "";
    ret = ln + "\n";
    unsigned int cbeg = loc.beg_column;
    unsigned int cend = loc.end_column;
    int count = cend - cbeg - 1;
    int space = cbeg;
    //multiline error just display the beginning
    if (loc.end_line != loc.beg_line)
      count = 1;
    space = space < 0 ? 0 : space;
    count = count < 1 ? 1 : count;
    for (int i = 0; i < space; ++i)
      ret += " ";
    for (int i = 0; i < count; ++i)
      ret += "^";
    ret += "\n";
    return ret;
  }

  //public interface
  ParseResult parse(const FileReaderPtr& file) {
    ParseResult ret;

    if (!file->isOpen()) {
      ret.messages.push_back(Message(MessageType_Error, "Can't open file", file->filename()));
      return ret;
    }
    Parser p(file);
    return p.result();
  }


}
