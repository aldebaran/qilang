/*
** qiLang generated file. DO NOT EDIT
*/
#pragma once
#ifndef YEAH_d768ca12_33e0_4447_94b4_151bbad67632
#define YEAH_d768ca12_33e0_4447_94b4_151bbad67632

#include <qitype/signal.hpp>
#include <qitype/property.hpp>
#include <qitype/anyobject.hpp>
#include <string>
#include <vector>
#include <map>

namespace pimpl {

class BarInterface {
public:
  virtual ~BarInterface() {}
  virtual void foo(const std::string&) = 0;
};

typedef qi::Object<BarInterface> BarObj;

//Bar newBar();
}

#endif
