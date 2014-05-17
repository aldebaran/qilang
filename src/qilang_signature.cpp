/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <iostream>
#include <cstring>
#include <qi/log.hpp>
#include <qilang/parser.hpp>
#include <qitype/signature.hpp>
#include <boost/algorithm/string.hpp>
#include <qitype/metaobject.hpp>

namespace qilang {

class QiLangSignatureConvertor {
public:
  TypeExprNodePtr visit(const qi::Signature& sig) {
    switch(sig.type()) {
      case '[':
        return visitList(sig);
        break;
      case '{':
        return visitMap(sig);
        break;
      case '(':
        return visitTuple(sig);
        break;
      default:
        return visitSimple(sig);
        break;
    }
  }

protected:
  TypeExprNodePtr visit(const qi::SignatureVector& elements) {
    TypeExprNodePtrVector elts;

    //create a tuple with each elements...
    qi::SignatureVector::const_iterator it;

    for (it = elements.begin(); it != elements.end(); ++it) {
      elts.push_back(visit(*it));
    }
    return boost::make_shared<qilang::TupleTypeExprNode>(elts, Location());
  }

  TypeExprNodePtr visitSimple(const qi::Signature& sig) {
    TypeExprNodePtr ret;
    switch(sig.type()) {
      case 'b':
        ret = boost::make_shared<qilang::BuiltinTypeExprNode>(BuiltinType_Bool, "bool", Location());
        break;
      case 'c':
        ret = boost::make_shared<qilang::BuiltinTypeExprNode>(BuiltinType_Int8, "int8", Location());
        break;
      case 'C':
        ret = boost::make_shared<qilang::BuiltinTypeExprNode>(BuiltinType_UInt8, "uint8", Location());
        break;
      case 'w':
        ret = boost::make_shared<qilang::BuiltinTypeExprNode>(BuiltinType_Int16, "int16", Location());
        break;
      case 'W':
        ret = boost::make_shared<qilang::BuiltinTypeExprNode>(BuiltinType_UInt16, "uint16", Location());
        break;
      case 'i':
        ret = boost::make_shared<qilang::BuiltinTypeExprNode>(BuiltinType_Int32, "int32", Location());
        break;
      case 'I':
        ret = boost::make_shared<qilang::BuiltinTypeExprNode>(BuiltinType_UInt32, "uint32", Location());
        break;
      case 'l':
        ret = boost::make_shared<qilang::BuiltinTypeExprNode>(BuiltinType_Int64, "int64", Location());
        break;
      case 'L':
        ret = boost::make_shared<qilang::BuiltinTypeExprNode>(BuiltinType_UInt64, "uint64", Location());
        break;
      case 'f':
        ret = boost::make_shared<qilang::BuiltinTypeExprNode>(BuiltinType_Float32, "float32", Location());
        break;
      case 'd':
        ret = boost::make_shared<qilang::BuiltinTypeExprNode>(BuiltinType_Float64, "float64", Location());
        break;
      case 'v':
        ret = boost::make_shared<qilang::BuiltinTypeExprNode>(BuiltinType_Nothing, "nothing", Location());
        break;
      case 's':
        ret = boost::make_shared<qilang::BuiltinTypeExprNode>(BuiltinType_String, "str", Location());
        break;
      case 'm':
        ret = boost::make_shared<qilang::BuiltinTypeExprNode>(BuiltinType_Value, "any", Location());
        break;
      case 'o':
        ret = boost::make_shared<qilang::BuiltinTypeExprNode>(BuiltinType_Object, "obj", Location());
        break;
      case 'X':
        ret = boost::make_shared<qilang::BuiltinTypeExprNode>(BuiltinType_Value, "any", Location());
        break;
      default:
        throw std::runtime_error("unhandled signature type conversion");
    }
    return ret;
  }

  TypeExprNodePtr visitList(const qi::Signature& sig) {
    return boost::make_shared<qilang::ListTypeExprNode>(visit(sig.children().at(0)), Location());
  }

  TypeExprNodePtr visitMap(const qi::Signature& sig) {
    TypeExprNodePtr ret;
    ret = boost::make_shared<qilang::MapTypeExprNode>(visit(sig.children().at(0)), visit(sig.children().at(1)), Location());
    return ret;
  }

  TypeExprNodePtr visitTuple(const qi::Signature &sig) {
    TypeExprNodePtrVector    elts;
    std::vector<std::string> annot;
    std::string              annotation(sig.annotation());

    boost::algorithm::split(annot, annotation, boost::algorithm::is_any_of(","));

    //this is a struct, just return it's name
    if (annot.size() && !annot[0].empty())
      return boost::make_shared<qilang::CustomTypeExprNode>(annot[0], Location());

    //this is a tuple
    return visit(sig.children());
  }

  bool                 _done;
  const qi::Signature *_sig;
  std::string          _result;


};

TypeExprNodePtr signatureToQiLang(const qi::Signature& sig) {
  QiLangSignatureConvertor qlsc;
  return qlsc.visit(sig);
}



}
