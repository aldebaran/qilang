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

  enum MessageType {
    MessageType_None = 0,
    MessageType_Error = 1,
    MessageType_Warning = 2,
    MessageType_Info = 3
  };

  class QILANG_API Message {
  public:
    Message()
    {}

    explicit Message(MessageType type, const std::string& what)
      : _type(type)
      , _what(what)
    {}
    explicit Message(MessageType type, const std::string& what, Location loc)
      : _type(type)
      , _what(what)
      , _loc(loc)
    {}

    MessageType  type() const           { return _type; }
    const char*  what() const           { return _what.c_str(); }
    const std::string& filename() const { return _loc.filename; }
    const Location&    loc() const      { return _loc; }
  protected:
    MessageType _type;
    std::string _what;
    Location    _loc;
  };

  typedef std::vector<Message> MessageVector;

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
    std::string   filename;
    std::string   package;
    NodePtrVector ast;
    MessageVector messages;

    bool hasError() const {
      return messages.size() > 0;
    }

    void printMessage(std::ostream& out) const;
  };

  QILANG_API ParseResult parse(const FileReaderPtr& filename);
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
