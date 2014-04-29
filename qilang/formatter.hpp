/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#ifndef   	VISITOR_HPP_
# define   	VISITOR_HPP_

#include <qilang/api.hpp>
#include <qilang/node.hpp>
#include <sstream>
#include <fstream>
#include <boost/make_shared.hpp>

namespace qilang {

  class PackageManager;
  class ParseResult;
  typedef boost::shared_ptr<PackageManager> PackageManagerPtr;

  class QILANG_API FileWriter {
  public:
    explicit FileWriter(const std::string& filename)
      : _filename(filename)
      , _fileout(filename.c_str())
      , _out(&_fileout)
    {}

    explicit FileWriter(std::ostream *out, const std::string& filename)
      : _filename(filename)
      , _out(out)
    {}

    bool isOpen()                       { return _out->good(); }
    const std::string& filename() const { return _filename; }
    std::ostream& out()                 { return *_out; }

  protected:
    std::string   _filename;
    std::ofstream _fileout;
    std::ostream* _out;
  };
  typedef boost::shared_ptr<FileWriter> FileWriterPtr;
  inline FileWriterPtr newFileWriter(const std::string& fname) { return boost::make_shared<FileWriter>(fname); }
  inline FileWriterPtr newFileWriter(std::ostream* o, const std::string& fname) { return boost::make_shared<FileWriter>(o, fname); }

  QILANG_API std::string genCppObjectInterface(const PackageManagerPtr& pm, const ParseResult& nodes);

  QILANG_API std::string genCppObjectRegistration(const PackageManagerPtr& pm, const ParseResult& nodes);

  QILANG_API std::string genCppObjectRemote(const PackageManagerPtr& pm, const ParseResult& nodes);

  QILANG_API std::string genCppObjectLocal(const PackageManagerPtr& pm, const ParseResult& nodes);

  QILANG_API std::string formatAST(const NodePtrVector& node);
  QILANG_API std::string format(const NodePtrVector& node);

  QILANG_API std::string formatAST(const NodePtr& node);
  QILANG_API std::string format(const NodePtr& node);

  QILANG_API qi::AnyValue toAnyValue(const NodePtr& node);

  enum FormatterCodeGen {
    QiLang,
    Cpp_Header,
    Cpp_Code
  };

  enum FormatterType {
    Expr,
    ConstExpr,
    TypeExpr,
    Data,
    Stmt,
  };

  class Formatter {
  public:
    explicit Formatter(const NodePtr& node);
    explicit Formatter(const NodePtrVector& node);

    // Runtime
    //qi::AnyValue formatValue();
    //qi::AnyType  formatType();
    //qi::AnyStmt  formatStmt();
    std::string  format(FormatterCodeGen codegen, FormatterType type);
  };

  /* Format Options:
   * language: ast, qilang, cpp
   *
   * formatValue(ConstExpr) -> AnyValue
   * formatExpr(Expr) std::string
   * format(Node)
   */
}

#endif	    /* !VISITOR_PP_ */
