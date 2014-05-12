/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#ifndef QILANG_PARSER_HPP
#define QILANG_PARSER_HPP

#include <qilang/api.hpp>
#include <fstream>
#include <qilang/node.hpp>
#include <boost/make_shared.hpp>

namespace qi {
class Signature;
class MetaObject;
};

namespace qilang {

  enum DiagnosticType {
    DiagnosticType_None = 0,
    DiagnosticType_Error = 1,
    DiagnosticType_Warning = 2,
    DiagnosticType_Info = 3
  };

  class QILANG_API Diagnostic {
  public:
    Diagnostic()
    {}

    explicit Diagnostic(DiagnosticType type, const std::string& what)
      : _type(type)
      , _what(what)
    {}
    explicit Diagnostic(DiagnosticType type, const std::string& what, Location loc)
      : _type(type)
      , _what(what)
      , _loc(loc)
    {}

    DiagnosticType  type() const           { return _type; }
    const char*  what() const           { return _what.c_str(); }
    const std::string& filename() const { return _loc.filename; }
    const Location&    loc() const      { return _loc; }

    void print(std::ostream &out) const;
  protected:
    DiagnosticType _type;
    std::string _what;
    Location    _loc;
  };

  typedef std::vector<Diagnostic> DiagnosticVector;

  class QILANG_API FileReader {
  public:
    explicit FileReader(const std::string& filename)
      : _filename(filename)
      , _filein(filename.c_str())
      , _in(&_filein)
    {}

    explicit FileReader(std::istream *in, const std::string& filename)
      : _filename(filename)
      , _in(in)
    {}

    bool isOpen()                       { return _in->good(); }
    const std::string& filename() const { return _filename; }
    std::istream& in()                  { return *_in; }

  protected:
    std::string   _filename;
    std::ifstream _filein;
    std::istream* _in;
  };

  typedef boost::shared_ptr<FileReader> FileReaderPtr;


  inline FileReaderPtr newFileReader(const std::string& fname) { return boost::make_shared<FileReader>(fname); }
  inline FileReaderPtr newFileReader(std::istream* in, const std::string& fname) { return boost::make_shared<FileReader>(in, fname); }

  class QILANG_API ParseResult {
  public:
    std::string      filename;
    std::string      package;
    NodePtrVector    ast;
    DiagnosticVector _messages;

    DiagnosticVector& messages() { return _messages; }

    void addDiag(const Diagnostic& diag) {
      _messages.push_back(diag);
      diag.print(std::cout);
    }

    bool hasError() const {
      return _messages.size() > 0;
    }

    void printMessage(std::ostream& out) const;
  };

  typedef boost::shared_ptr<ParseResult> ParseResultPtr;

  inline ParseResultPtr newParseResult() { return boost::make_shared<ParseResult>(); }

  QILANG_API ParseResultPtr parse(const FileReaderPtr& filename);
  QILANG_API TypeExprNodePtr signatureToQiLang(const qi::Signature& sig);
  QILANG_API NodePtr metaObjectToQiLang(const std::string& name, const qi::MetaObject& obj);

  /* parse options:
   * - interface
   * - const expr / data
   * - expr
   * - object
   *
   * From: qilang file, AnyValue (for value), ...
   */
}

#endif // QIPARSER_HPP
