/*
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <iostream>
#include <sstream>
#include <cstring>
#include <qi/log.hpp>
#include <qilang/parser.hpp>
#include <qi/signature.hpp>
#include <boost/algorithm/string.hpp>
#include <qi/type/metaobject.hpp>

namespace qilang {

  static ParamFieldDeclNodePtrVector sigToParams(qi::MetaMethodParameterVector parametersName, const qi::Signature& sig) {
    ParamFieldDeclNodePtrVector params;

    if (sig == qi::Signature("m")) {
      params.push_back(boost::make_shared<qilang::ParamFieldDeclNode>("args", ParamFieldType_VarArgs, Location()));
      return params;
    }

    qi::Signature              signature = sig;
    const qi::SignatureVector& vsig = signature.children();
    qi::SignatureVector::const_iterator it;
    qi::MetaMethodParameterVector::const_iterator itpn;

    itpn = parametersName.begin();
    for (it = vsig.begin(); it != vsig.end(); ++it) {
      if (itpn == parametersName.end()){
        params.push_back(boost::make_shared<ParamFieldDeclNode>("_", signatureToQiLang(*it), Location()));
      }
      else {
        params.push_back(boost::make_shared<ParamFieldDeclNode>((*itpn).name(), signatureToQiLang(*it), Location()));
      }
      if (itpn != parametersName.end())
      {
        ++itpn;
      }
    }
    return params;
  }

  static ParamFieldDeclNodePtrVector sigToParams(const qi::Signature& sig) {
    ParamFieldDeclNodePtrVector params;

    if (sig == qi::Signature("m")) {
      params.push_back(boost::make_shared<qilang::ParamFieldDeclNode>("args", ParamFieldType_VarArgs, Location()));
      return params;
    }

    qi::Signature              signature = sig;
    const qi::SignatureVector& vsig = signature.children();

    qi::SignatureVector::const_iterator it;
    unsigned i = 0;
    for (it = vsig.begin(); it != vsig.end(); ++it, ++i) {
      //TODO: handled named field

      params.push_back(boost::make_shared<ParamFieldDeclNode>("_", signatureToQiLang(*it), Location()));
    }

    return params;
  }

  static CommentNodePtr commentToQiLang(const qi::MetaMethod& meth) {
    std::string                   commentsOut;
    std::stringstream             commentsOutS;
    qi::MetaMethodParameterVector parameters;
    StringVector                  lines;
    std::string                   line;
    qi::MetaMethodParameterVector::const_iterator it;

    parameters = meth.parameters();
    line = meth.description();

    boost::split(lines, line, boost::is_any_of("\r\n"));
    commentsOutS << "# \\brief " << lines.at(0);
    for ( unsigned int i = 1; i < lines.size(); ++i) {
      commentsOutS << "\n  #" << lines.at(i);
    }
    for (it = parameters.begin(); it != parameters.end(); ++it) {
      line = (*it).description();
      boost::split(lines, line, boost::is_any_of("\r\n"));
      commentsOutS << "\n  # \\param " <<(*it).name() << ": " << lines.at(0);
      for ( unsigned int i = 1; i < lines.size(); ++i) {
        commentsOutS << "\n  # " << lines.at(i);
      }
    }
    if(  std::strlen(meth.returnDescription().c_str()) != 0 )
    {
      line = meth.returnDescription();
      boost::split(lines, line, boost::is_any_of("\r\n"));

      commentsOutS << "\n  # \\return " << lines.at(0);
      for ( unsigned int i = 1; i < lines.size(); ++i) {
        commentsOutS << "\n  # " << lines.at(i);
      }
    }
    commentsOut = commentsOutS.str();

    return boost::make_shared<qilang::CommentNode>(commentsOut, Location());
  }

  static DeclNodePtr metaMethodToQiLang(const qi::MetaMethod& meth) {
    CommentNodePtr                 comment;
    ParamFieldDeclNodePtrVector    params;
    qi::MetaMethodParameterVector  parametersName;
    TypeExprNodePtr                ret;

    parametersName = meth.parameters();
    comment = commentToQiLang(meth);
    params = sigToParams(parametersName, meth.parametersSignature());
    ret = signatureToQiLang(meth.returnSignature());

    return boost::make_shared<qilang::FnDeclNode>(meth.name(), comment, params, ret, Location());
  }

  static DeclNodePtr metaSignalToQiLang(const qi::MetaSignal& sig){
    ParamFieldDeclNodePtrVector params;

    params = sigToParams(sig.parametersSignature());
    return boost::make_shared<qilang::EmitDeclNode>(sig.name(), params, Location());
  }

  static DeclNodePtr metaPropertyToQiLang(const qi::MetaProperty& prop){
    ParamFieldDeclNodePtrVector params;

    params = sigToParams(prop.signature());
    return boost::make_shared<qilang::PropDeclNode>(prop.name(), params, Location());
  }


  NodePtr metaObjectToQiLang(const std::string& name, const qi::MetaObject& obj) {
    DeclNodePtrVector declarations;

    //foreach method do convert
    qi::MetaObject::MethodMap                 methods = obj.methodMap();
    qi::MetaObject::MethodMap::const_iterator itMM;

    for (itMM = methods.begin(); itMM != methods.end(); ++itMM) {
      if(itMM->second.uid() >= 100)
        declarations.push_back(metaMethodToQiLang(itMM->second));
    }

    //foreach signal do convert
    qi::MetaObject::SignalMap                 signas = obj.signalMap();
    qi::MetaObject::SignalMap::const_iterator itSM;

    for (itSM = signas.begin(); itSM != signas.end(); ++itSM) {
      if(itSM->second.uid() >= 100)
        declarations.push_back(metaSignalToQiLang(itSM->second));
    }

    //foreach property do convert
    qi::MetaObject::PropertyMap                 propertys = obj.propertyMap();
    qi::MetaObject::PropertyMap::const_iterator itPM;

    for (itPM = propertys.begin(); itPM != propertys.end(); ++itPM) {
      if(itPM->second.uid() >= 100)
        declarations.push_back(metaPropertyToQiLang(itPM->second));
    }

    return boost::make_shared<qilang::InterfaceDeclNode>(name, declarations, Location());
  }

}
