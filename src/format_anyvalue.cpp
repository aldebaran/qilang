/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <qilang/formatter.hpp>
#include <qilang/node.hpp>
#include <qi/anyvalue.hpp>

namespace qilang {
  /**
 * @brief The DataToAnyValueVisitor class
 * convert data to an AnyValue...
 *
 */
  class DataToAnyValueVisitor : public LiteralNodeVisitor {
  public:
    qi::AnyValue av;

    qi::AnyValue convert(const LiteralNodePtr& node)
    {
      av.reset();
      acceptData(node);
      return av;
    }

    virtual void acceptData(const LiteralNodePtr &node)      { node->accept(this); }

    /** Convert Data to AnyValue
   */
    /// to put into anyvaluedata.hpp/cpp  (its generic)
    void visitData(BoolLiteralNode *node) {
      av = qi::AnyValue::from(node->value);
    }
    void visitData(IntLiteralNode *node) {
      av = qi::AnyValue::from(node->value);
    }
    void visitData(FloatLiteralNode *node) {
      av = qi::AnyValue::from(node->value);
    }
    void visitData(StringLiteralNode *node) {
      av = qi::AnyValue::from(node->value);
    }

    void visitData(TupleLiteralNode* node) {
      std::cout << "TupleConstNode visitor" << std::endl;
      throw std::runtime_error("not implemented");
    }
    void visitData(ListLiteralNode* node) {
      std::cout << "ListConstNode visitor" << std::endl;
      throw std::runtime_error("not implemented");
    }
    void visitData(DictLiteralNode* node) {
      std::cout << "DictConstNode visitor" << std::endl;
      throw std::runtime_error("not implemented");
    }

  };

  qi::AnyValue toAnyValue(const NodePtr& node) {
    if (node->kind() == NodeKind_Literal)
      return DataToAnyValueVisitor().convert(boost::static_pointer_cast<LiteralNode>(node));
    throw std::runtime_error("bad node kind");
  }

}
