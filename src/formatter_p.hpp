/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#ifndef   	FORMATTER_P_HPP_
# define   	FORMATTER_P_HPP_

#include <qilang/node.hpp>

namespace std {
  //only to avoid mistake... (shared_ptr are displayed as pointer by default...)
  //this function will generate an error instead
  template <class T>
  std::ostream& operator<<(std::ostream&o, const boost::shared_ptr<T>& node) {
    o << *node.get();
    return o;
  }
}

namespace qilang {

  class FormatAttr {
  public:
    FormatAttr(bool active = false)
      : _active(active)
      , _block(0)
    {}

    void block() { _block++; }
    void unblock() { _block--; _block = _block < 0 ? 0 : _block; }

    void activate()    { _active++; }
    void desactivate() { _active--; _active = _active < 0 ? 0 : _active; }

    bool isActive() const { return _active > 0 && _block == 0; }
    const std::string &operator()(const std::string& str) const {
      static std::string empt;
      if (isActive())
        return str;
      else
        return empt;
    }

  protected:
    std::string _name;
    int         _active;
    int         _block;
  };

  class ScopedFormatAttrBlock {
  public:
    ScopedFormatAttrBlock(FormatAttr& attr)
      : _attr(attr)
    {
      _attr.block();
    }

    ~ScopedFormatAttrBlock()
    {
      _attr.unblock();
    }
  protected:
    FormatAttr& _attr;
  };

  class ScopedFormatAttrActivate {
  public:
    ScopedFormatAttrActivate(FormatAttr& attr)
      : _attr(attr)
    {
      _attr.activate();
    }

    ~ScopedFormatAttrActivate()
    {
      _attr.desactivate();
    }
  protected:
    FormatAttr& _attr;
  };


  class BasicNodeFormatter {
  public:
    //virtual void accept(const NodePtr& node) = 0;
    std::stringstream &out() {
      return _ss;
    }
  private:
    std::stringstream _ss;
  };

  class LiteralNodeFormatter : virtual public BasicNodeFormatter, public LiteralNodeVisitor {
  };

  class ExprNodeFormatter : virtual public BasicNodeFormatter, public ExprNodeVisitor {
  };

  class TypeExprNodeFormatter : virtual public BasicNodeFormatter, public TypeExprNodeVisitor {
  };

  /**
   * @brief The NodeFormatter class
   *
   * to implement a formatter you need to implement the virtual accept method.
   *
   * use out() for expressions
   * use indent() for statements
   */
  class IndentNodeFormatter : virtual public BasicNodeFormatter {
  public:
    virtual void acceptStmt(const StmtNodePtr& node) = 0;

    IndentNodeFormatter()
      : _indent(0)
    {}

    class ScopedIndent {
    public:
      ScopedIndent(int& indent, int add = 2)
        : _indent(indent)
        , _add(add)
      {
        _indent += _add;
      }
      ~ScopedIndent() { _indent -= _add; }
    private:
      int &_indent;
      int  _add;
    };

    virtual void formatHeader() {};
    virtual void formatFooter() {};

  public:
    std::stringstream &indent(int changes = 0) {
      _indent += changes;
      if (_indent < 0)
        _indent = 0;
      for (int i = 0; i < _indent; ++i) {
        out().put(' ');
      }
      return out();
    }

    //indented block

  public:
    int               _indent;
  };

  class StmtNodeFormatter : virtual public IndentNodeFormatter, public StmtNodeVisitor {
  public:
    virtual void acceptStmt(const StmtNodePtr& node) = 0;

//    void stmt(StmtNodePtr node) {
//      static const std::string ret;
//      acceptStmt(node);
//      return ret;
//    }

    void scopedStmt(const qilang::StmtNodePtrVector& vec) {
      ScopedIndent _(_indent);
      for (unsigned int i = 0; i < vec.size(); ++i) {
        acceptStmt(vec[i]);
      }
    }
  };

  class DeclNodeFormatter : virtual public IndentNodeFormatter, public DeclNodeVisitor {
  public:
    void scopedDecl(const qilang::DeclNodePtrVector& vec) {
      ScopedIndent _(_indent);
      for (unsigned int i = 0; i < vec.size(); ++i) {
        acceptDecl(vec[i]);
      }
    }
    void scopedField(const qilang::FieldDeclNodePtrVector& vec) {
      ScopedIndent _(_indent);
      for (unsigned int i = 0; i < vec.size(); ++i) {
        acceptDecl(vec[i]);
      }
    }

  };

  class FileFormatter: virtual public BasicNodeFormatter {
  public:
    virtual void accept(const NodePtr& node) = 0;

    virtual void formatHeader() {}
    virtual void formatFooter() {}

    virtual std::string format(const NodePtrVector& node) {
      formatHeader();
      for (unsigned int i = 0; i < node.size(); ++i) {
        if (!node.at(i))
          throw std::runtime_error("Invalid Node");
        accept(node.at(i));
      }
      formatFooter();
      return out().str();
    }

    virtual std::string format(const NodePtr& node) {
      if (!node)
        throw std::runtime_error("Invalid Node");
      formatHeader();
      accept(node);
      formatFooter();
      return out().str();
    }
  };

}

#endif	    /* !FORMATTER_P_PP_ */
