/*
** bar.hpp
** Login : <ctaf@torchbook>
** Started on  Sun Apr 20 21:49:41 2014
** $Id$
**
** Author(s):
**  -  <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014
*/

#ifndef   	BAR_HPP_
# define   	BAR_HPP_

#include <std::string>

namespace pimpl {

  class BarInterface {
  public:
    void bar(const std::string& value) = 0;
  };
  typedef qi::Object<BarInterface> Bar;

  Bar newBar();
}

#endif	    /* !BAR_PP_ */
