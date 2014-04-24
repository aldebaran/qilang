#!/bin/sh
##
## gen.sh
## Login : <ctaf@cgestes-de.aldebaran.lan>
## Started on  Wed Apr 23 16:17:47 2014
## $Id$
##
## Author(s):
##  -  <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2014
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
##


qic -c cppi pimpl/bar.idl.qi -o pimpl/bar.hpp
cat pimpl/bar.hpp
echo ""

qic -c cppr pimpl/bar.idl.qi -o src/bar.cpp
cat src/bar.cpp
echo ""
