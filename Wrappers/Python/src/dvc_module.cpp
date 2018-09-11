/*
   This work is part of the Core Imaging Library developed by
   Visual Analytics and Imaging System Group of the Science Technology
   Facilities Council, STFC

   Copyright 2017 Srikanth Nagella, Edoardo Pasca

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

#include <iostream>
#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <cmath>

//#include "boost/tuple/tuple.hpp"
#include "Utility.h"
#include "Point.h"

#if defined(_WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#include <windows.h>
// this trick only if compiler is MSVC
__if_not_exists(uint8_t) { typedef __int8 uint8_t; }
__if_not_exists(uint16_t) { typedef __int8 uint16_t; }
#endif

namespace bp = boost::python;
namespace np = boost::python::numpy;



BOOST_PYTHON_MODULE(dvc)
{
	np::initialize();

	//To specify that this module is a package
	bp::object package = bp::scope();
	package.attr("__path__") = "dvc";
	
	np::dtype dt1 = np::dtype::get_builtin<uint8_t>();
	np::dtype dt2 = np::dtype::get_builtin<uint16_t>();
	bp::class_<Point>("Point", bp::init<double, double, double>())
		.def("move_to", &Point::move_to)
		.def("move_by", &Point::move_by)
		.def("pt_dist", &Point::pt_dist)
		.def("x",       &Point::x)
		.def("y",       &Point::y)
		.def("z",       &Point::z)
		.def("ix",      &Point::ix)
		.def("iy",      &Point::iy)
		.def("iz",      &Point::iz)
		;
		//.def("SingleUncollapse", &CilContourTreeSegmentation::SingleUnCollapse)
		//.def("SingleCollapse", &CilContourTreeSegmentation::SingleCollapse)
		
}

