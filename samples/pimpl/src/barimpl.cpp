/*
** barimpl.cpp
** Login : <ctaf@torchbook>
** Started on  Sun Apr 20 21:45:18 2014
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

#include <iostream>
#include <pimpl/api.hpp>
#include <pimpl/bar.hpp>

class PIMPL_API BarImpl : public pimpl::BarInterface {
public:
  void foo(const std::string& value) {
    std::cout << "foo:" << value << std::endl;
  }
};

//QI_REGISTER_IMPLEMENTATION(pimpl::BarInterface, BarImpl);

pimpl::Bar pimpl::newBar() {
  return pimpl::Bar(new BarImpl);
}


//Declare the content of the pimpl package
//QI_PACKAGE(pimpl) {
  //registerImpl<BarImpl, Bar>();
  //registerRemote<BarRemoteImpl, Bar>();  //object that take a session?
//}
