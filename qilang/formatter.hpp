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

namespace qilang {

  QILANG_API std::string genCppObjectInterface(const NodePtr& node);
  QILANG_API std::string genCppObjectInterface(const NodePtrVector& nodes);

  QILANG_API std::string genCppObjectRegistration(const NodePtr& node);
  QILANG_API std::string genCppObjectRegistration(const NodePtrVector& nodes);

  QILANG_API std::string formatAST(const NodePtrVector& node);
  QILANG_API std::string format(const NodePtrVector& node);

  QILANG_API std::string formatAST(const NodePtr& node);
  QILANG_API std::string format(const NodePtr& node);

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
  }
  fn newFormatter(node qilang.Node)
  fn newFormatter(nodes []qilang.Node)

  /* Format Options:
   * language: ast, qilang, cpp
   *
   * formatValue(ConstExpr) -> AnyValue
   * formatExpr(Expr) std::string
   * format(Node)
   */
}

#endif	    /* !VISITOR_PP_ */
