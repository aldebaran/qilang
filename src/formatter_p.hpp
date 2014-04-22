/*
** formatter_p.hpp
** Login : <ctaf@torchbook>
** Started on  Wed Apr  9 14:14:10 2014
** $Id$
**
** Author(s):
**  -  <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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


  /**
   * @brief The NodeFormatter class
   *
   * to implement a formatter you need to implement the virtual accept method.
   *
   * use out() for expressions
   * use indent() for statements
   */
  class NodeFormatter {
  public:
    NodeFormatter()
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

    //virtual std::string format(const NodePtrVector& node) = 0;
    //virtual std::string format(const NodePtr& node) = 0;

    virtual std::string format(const NodePtrVector& node) {
      formatHeader();
      for (int i = 0; i < node.size(); ++i) {
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

    virtual void accept(const NodePtr& node) = 0;
    virtual void formatHeader() {};
    virtual void formatFooter() {};



  public:
    std::stringstream &indent(int changes = 0) {
      _indent += changes;
      if (_indent < 0)
        _indent = 0;
      for (int i = 0; i < _indent; ++i) {
        _ss.put(' ');
      }
      return _ss;
    }

    std::stringstream &out() {
      return _ss;
    }

    //visit a node
    // TODO check node is an EXPR node
    const std::string& expr(NodePtr node) {
      static const std::string ret;
      accept(node);
      return ret;
    }

  public:
    int               _indent;

  private:
    std::stringstream _ss;
  };

}

#endif	    /* !FORMATTER_P_PP_ */
