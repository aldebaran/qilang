/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#ifndef   	FORMATTER_P_HPP_
# define   	FORMATTER_P_HPP_

#include <qilang/node.hpp>
#include <qilang/packagemanager.hpp>

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
    BasicNodeFormatter()
      : _ss(_ssi)
    {}
    explicit BasicNodeFormatter(std::stringstream& ss)
      : _ss(ss)
    {}

    std::stringstream &out() {
      return _ss;
    }

  private:
    std::stringstream& _ss;
    std::stringstream _ssi;
  };

  /**
   * @brief The NodeFormatter class
   *
   * to implement a formatter you need to implement the virtual accept method.
   *
   * use out() for expressions
   * use indent() for statements
   */
  class IndentNodeFormatter : public BasicNodeFormatter {
  public:

    IndentNodeFormatter()
      : _indent(0)
    {}
    explicit IndentNodeFormatter(std::stringstream& ss, int _indent)
      : BasicNodeFormatter(ss)
      , _indent(_indent)
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

  template <typename B = NodeVisitor>
  class NodeFormatter : public IndentNodeFormatter, public B {
  public:
    explicit NodeFormatter(std::stringstream& ss, int indent)
      : IndentNodeFormatter(ss, indent)
    {}
    NodeFormatter()
    {}

    template <typename T>
    void join(const std::vector< boost::shared_ptr<T> >& vec, const std::string& sep) {
      for (unsigned int i = 0; i < vec.size(); ++i) {
        this->accept(vec.at(i));
        if (i + 1 < vec.size())
          this->out() << sep;
      }
    }

    template <typename T>
    void join(const std::vector<T>& vals, const std::string& sep) {
      for (unsigned i = 0; i < vals.size(); ++i) {
        this->out() << vals.at(i);
        if (i + 1 < vals.size())
          this->out() << sep;
      }
    }

    //assuming T is a shared_ptr<SomeNode>
    template <typename T>
    void scoped(const std::vector< boost::shared_ptr<T> >& vec) {
      ScopedIndent _(_indent);
      for (unsigned int i = 0; i < vec.size(); ++i) {
        this->accept(vec.at(i));
      }
    }

    virtual void formatHeader() {}
    virtual void formatFooter() {}

    virtual std::string format(const NodePtrVector& node) {
      formatHeader();
      for (unsigned int i = 0; i < node.size(); ++i) {
        if (!node.at(i))
          throw std::runtime_error("Invalid Node");
        this->accept(node.at(i));
      }
      formatFooter();
      return this->out().str();
    }

    virtual std::string format(const NodePtr& node) {
      if (!node)
        throw std::runtime_error("Invalid Node");
      formatHeader();
      this->accept(node);
      formatFooter();
      return this->out().str();
    }
  };

  class ScopedNamespaceEscaper {
  public:
    ScopedNamespaceEscaper(std::ostream& out, const StringVector& ns)
      : out(out)
      , currentNs(ns)
    {
      for (size_t i = 0; i < currentNs.size(); ++i) {
        out << "}" << std::endl;
      }
      out << std::endl;
    }

    ~ScopedNamespaceEscaper() {
      unsigned int indent = 0;
      for (unsigned int i = 0; i < currentNs.size(); ++i) {
        for (unsigned int j = 0; j < indent; ++j) {
          out << "  ";
        }
        out << "namespace " << currentNs.at(i) << " {" << std::endl;
        indent += 1;
      }
      out << std::endl;
    }

    std::ostream& out;
    StringVector currentNs;
  };

  std::string qiLangToCppInclude(const PackagePtr& pkg, const std::string& filename);

}

#endif	    /* !FORMATTER_P_PP_ */
