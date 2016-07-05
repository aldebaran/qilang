/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <qilang/formatter.hpp>
#include <qilang/node.hpp>
#include <qilang/visitor.hpp>

#include <qi/anyvalue.hpp>

namespace qilang {
  /**
   * @brief The DataToAnyValueVisitor class
   * convert data to an AnyValue...
   *
   */
  class DataToAnyValueVisitor : public DefaultNodeVisitor {
  public:
    qi::AnyValue av;

    qi::AnyValue convert(const LiteralNodePtr& node)
    {
      av.reset();
      accept(node);
      return av;
    }

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
    void visitData(TupleLiteralNode*) {
      throw std::runtime_error("not implemented");
    }

    void visitData(ListLiteralNode* node) {
      qi::AnyValueVector vect;
      for (std::vector<boost::shared_ptr<qilang::LiteralNode> >::iterator v = node->values.begin();
           v != node->values.end();
           ++v)
      {
        qi::AnyValue tmp = convert(*v);
        vect.push_back(tmp);
      }
      av = qi::AnyValue::from(vect);
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
