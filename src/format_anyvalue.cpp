/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <qilang/formatter.hpp>
#include <qilang/node.hpp>
#include <qitype/anyvalue.hpp>

namespace qilang {
  /**
 * @brief The DataToAnyValueVisitor class
 * convert data to an AnyValue...
 *
 */
  class DataToAnyValueVisitor : public ConstDataNodeVisitor {
  public:
    qi::AnyValue av;

    qi::AnyValue convert(const ConstDataNodePtr& node)
    {
      av.reset();
      acceptData(node);
      return av;
    }

    virtual void acceptData(const ConstDataNodePtr &node)      { node->accept(this); }

    /** Convert Data to AnyValue
   */
    /// to put into anyvaluedata.hpp/cpp  (its generic)
    void visitData(BoolConstDataNode *node) {
      av = qi::AnyValue::from(node->value);
    }
    void visitData(IntConstDataNode *node) {
      av = qi::AnyValue::from(node->value);
    }
    void visitData(FloatConstDataNode *node) {
      av = qi::AnyValue::from(node->value);
    }
    void visitData(StringConstDataNode *node) {
      av = qi::AnyValue::from(node->value);
    }

    void visitData(TupleConstDataNode* node) {
      std::cout << "TupleConstNode visitor" << std::endl;
      throw std::runtime_error("not implemented");
    }
    void visitData(ListConstDataNode* node) {
      std::cout << "ListConstNode visitor" << std::endl;
      throw std::runtime_error("not implemented");
    }
    void visitData(DictConstDataNode* node) {
      std::cout << "DictConstNode visitor" << std::endl;
      throw std::runtime_error("not implemented");
    }

  };

  qi::AnyValue toAnyValue(const NodePtr& node) {
    if (node->kind() == NodeKind_ConstData)
      return DataToAnyValueVisitor().convert(boost::static_pointer_cast<ConstDataNode>(node));
    throw std::runtime_error("bad node kind");
  }

}
