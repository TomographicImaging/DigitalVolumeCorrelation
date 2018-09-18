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
#include <cmath>
#include <fstream> 
#include <vector>
#include <string>
#include <iostream>
#include <string>
#include <iostream>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/enum.hpp>

//#include "boost/tuple/tuple.hpp"
//#include "npy/parse.h"
#include "Utility.h"
#include "Point.h"
#include "DataCloud.h"
#include "Search.h"

#include "dvc_cmd.h"


#if defined(_WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#include <windows.h>
// this trick only if compiler is MSVC
__if_not_exists(uint8_t) { typedef __int8 uint8_t; }
__if_not_exists(uint16_t) { typedef __int8 uint16_t; }
#endif

namespace bp = boost::python;
namespace np = boost::python::numpy;

int loadPointCloudFromNumpy(DataCloud * dc, np::ndarray pointcloud) {
	// load the point cloud in the DataCloud from a Numpy Array
	int npoints = pointcloud.shape(0);
	Point a_point(0.0, 0.0, 0.0);
	int a_label;
	double ptx, pty, ptz;
	for (long i = 0; i < npoints; i++) {
		
		ptx = (double)bp::extract<double>(pointcloud[i][1]);
		pty = (double)bp::extract<double>(pointcloud[i][2]);
		ptz = (double)bp::extract<double>(pointcloud[i][3]);

		dc->points.push_back(a_point);
		dc->points[i].move_to(ptx, pty, ptz);
		dc->labels.push_back(a_label);
		a_label = (int)bp::extract<double>(pointcloud[i][0]);
		dc->labels[i] = a_label;
	}
    //c_value = (unsigned char)bp::extract<int>(pointcloud[index]);
	return npoints;
}

np::ndarray getPoint(DataCloud * dc, int index) {
	// get a specific point in the DataCloud
	double data[] = { 0,0,0,0 };
	Py_intptr_t shape[1] = { 4 };
	np::ndarray result = np::zeros(1, shape, np::dtype::get_builtin<double>());
	if (index < dc->points.size()) {
		data[0] = (double)dc->labels[index];
		data[1] = dc->points[index].x();
		data[2] = dc->points[index].y();
		data[3] = dc->points[index].z();
	}
	std::copy(data, data+shape[0], reinterpret_cast<double*>(result.get_data()));
	return result;
}

np::ndarray get_rigid_trans(RunControl * rc){
	double data[] = { 0,0,0 };
	Py_intptr_t shape[1] = { 3 };
	np::ndarray result = np::zeros(1, shape, np::dtype::get_builtin<double>());
	if (rc->rigid_trans.size() == 0)
		rc->rigid_trans.resize(3, 0.0);
	data[0] = rc->rigid_trans[0];
	data[1] = rc->rigid_trans[1];
	data[2] = rc->rigid_trans[2];
	
	std::copy(data, data + shape[0], reinterpret_cast<double*>(result.get_data()));
	return result;
}

void set_rigid_trans(RunControl * rc, np::ndarray rt) {
	rc->rigid_trans.resize(3, 0.0);
	rc->rigid_trans[0] = (double)bp::extract<double>(rt[0]);
	rc->rigid_trans[1] = (double)bp::extract<double>(rt[1]);
	rc->rigid_trans[2] = (double)bp::extract<double>(rt[2]);
}

np::ndarray get_subvol_aspect(RunControl * rc) {
	double data[] = { 0,0,0 };
	Py_intptr_t shape[1] = { 3 };
	np::ndarray result = np::zeros(1, shape, np::dtype::get_builtin<double>());
	if (rc->subvol_aspect.size() == 0)
		rc->subvol_aspect.resize(3, 1.0);
	data[0] = rc->subvol_aspect[0];
	data[1] = rc->subvol_aspect[1];
	data[2] = rc->subvol_aspect[2];

	std::copy(data, data + shape[0], reinterpret_cast<double*>(result.get_data()));
	return result;
}

void set_subvol_aspect(RunControl * rc, np::ndarray rt) {
	rc->subvol_aspect.resize(3, 1.0);
	rc->subvol_aspect[0] = (double)bp::extract<double>(rt[0]);
	rc->subvol_aspect[1] = (double)bp::extract<double>(rt[1]);
	rc->subvol_aspect[2] = (double)bp::extract<double>(rt[2]);
}

bp::list wparse_npy(RunControl * rc) {

	std::ifstream ifs;

	// C++11
	//ifs.open(rc->ref_fname, std::ifstream::in );
	ifs.open(rc->ref_fname.c_str(), std::ifstream::in );
	char magic[6] ;
	char major, minor;
	int offset = 0, header_length=0;
	char header_v1[2];
	char header_v2[4];
	ifs.read(magic, 6);
	offset += 6;
	//std::cout << "Magic string: <" << magic << ">" << std::endl;
	ifs.read(&major, 1);
	offset++;
	ifs.read(&minor, 1);
	offset++;

	//std::cout << "Version: " << (short)(major) << "." << (short)(minor) << std::endl;
	
	if (major == 1) {
		ifs.read(header_v1, 2);
		//std::cout << "header: " << *(reinterpret_cast<unsigned short*>(header_v1)) << std::endl;
		offset += 2;
		header_length = *(reinterpret_cast<unsigned short*>(header_v1));
	}
	else if (major == 2)
	{
		ifs.read(header_v2, 4);
		//std::cout << "header: " << *(reinterpret_cast<unsigned int*>(header_v2)) << std::endl;
		offset += 4;
		header_length = *(reinterpret_cast<unsigned int*>(header_v2));
	}

	offset += header_length;
	int j = 0;
	char *the_header;
	the_header = (char *)malloc(header_length * sizeof(char));
	std::vector<std::string> keys;
	std::string key = std::string();
	std::vector<std::string> values;
	std::string val = std::string();
	int parsing = 0;
	int parse_key = 0; int parse_value = 0;

	while (j < header_length) {
		char c = ifs.get();
		the_header[j] = c;
		std::cout << "read character " << c << std::endl;

		if (strcmp(&c, "{") && parse_key == 0 && parse_value == 0) {
			// it means we are at the beginning
			parse_key = 1;
			parse_value = 0;
		} 
		
		if (parse_key == 1) {
			std::cout << "parse_key " << c << std::endl;
			if (strcmp(&c, " ")) {
				std::cout << "skip" << std::endl;
			}
			else if (strcmp(&c, "'")) {
				if (parsing == 0) {
					std::cout << "parse_key parsing  0 -> 1" << std::endl;
					parsing = 1;
				}
				else {
					std::cout << "parse_key parsing  1 -> 0" << std::endl;
					parsing = 0;
					keys.push_back(key);
					std::cout << "key = " << key.size() << " <> " << std::endl;
					key.clear();
				}
			}
			else if (strcmp(&c, ":")) {
				std::cout << "parse_key -> 0 parse_value -> 1" << std::endl;
				parse_key = 0;
				parse_value = 1;
			}
			else {
				std::cout << "append char" << std::endl;
				key.append(1, c);
			}
		}
		if (parse_value == 1) {
			if (strcmp(&c, " ")) {
				//skip
			}
			else if (strcmp(&c, "'")) {
				if (parsing == 0) {
					parsing = 1;
				}
				else {
					parsing = 0;
					values.push_back(val);
					std::cout << "val = " << val << std::endl;
					val.clear();
				}
			}
			else {
				val.append(1, c);
			}
		}

		j++;
	}
	std::cout << std::endl;
/*
	for (auto val : values) {
		std::cout << "val " << val << std::endl;
	}
	for (auto key : keys) {
		std::cout << "key " << key << std::endl;
	}
*/	
	ifs.close();
	// pass the header to Python as it is easier to handle text
	// TODO: handle it here
	// C++11 
	//std::string out(the_header);
	std::string out;
	out.assign(the_header, header_length);
	free(the_header);

	bp::list result;
	result.append<std::string>(out);
	result.append<int>(offset);
	
	return result;
}

BOOST_PYTHON_MODULE(dvcw)
{
	np::initialize();

	//To specify that this module is a package
	bp::object package = bp::scope();
	package.attr("__path__") = "dvcw";
	
	np::dtype dt1 = np::dtype::get_builtin<uint8_t>();
	np::dtype dt2 = np::dtype::get_builtin<uint16_t>();
	/*Point class*/
	bp::class_<Point>("Point", bp::init<double, double, double>())
		.def("move_to",     &Point::move_to)
		.def("move_by",     &Point::move_by)
		.def("pt_dist",     &Point::pt_dist)
		.def("pt_encl_vol", &Point::pt_encl_vol)
		.def("x",           &Point::x)
		.def("y",           &Point::y)
		.def("z",           &Point::z)
		.def("ix",          &Point::ix)
		.def("iy",          &Point::iy)
		.def("iz",          &Point::iz)
		.def("rx",          &Point::rx)
		.def("ry",          &Point::ry)
		.def("rz",          &Point::rz)
		;
	
	/*****************************RunControl***********************************/
	bp::class_<RunControl>("RunControl")
		.def_readwrite("ref_fname",     &RunControl::ref_fname)
		.def_readwrite("cor_fname",     &RunControl::cor_fname)
		.def_readwrite("pts_fname",     &RunControl::pts_fname)
		.def_readwrite("out_fname",     &RunControl::out_fname)
		.def_readwrite("res_fname",     &RunControl::res_fname)
		.def_readwrite("sta_fname",     &RunControl::sta_fname)

		.def_readwrite("vol_bit_depth", &RunControl::vol_bit_depth)
		.def_readwrite("vol_endian",    &RunControl::vol_endian)
		.def_readwrite("vol_hdr_lngth", &RunControl::vol_hdr_lngth)
		.def_readwrite("vol_wide",      &RunControl::vol_wide)
		.def_readwrite("vol_high",      &RunControl::vol_high)
		.def_readwrite("vol_tall",      &RunControl::vol_tall)

		.def_readwrite("subvol_geom", &RunControl::subvol_geom)
		.def_readwrite("subvol_size", &RunControl::subvol_size)
		.def_readwrite("subvol_npts", &RunControl::subvol_npts)

		.def_readwrite("subvol_thresh",   &RunControl::subvol_thresh)
		.def_readwrite("gray_thresh_min", &RunControl::gray_thresh_min)
		.def_readwrite("gray_thresh_max", &RunControl::gray_thresh_max)
		.def_readwrite("min_vol_fract",   &RunControl::min_vol_fract)

		.def_readwrite("disp_max",     &RunControl::disp_max)
		.def_readwrite("num_srch_dof", &RunControl::num_srch_dof)

		.def_readwrite("obj_fcn", &RunControl::obj_fcn)
		.def_readwrite("int_typ", &RunControl::int_typ)
		.def_readwrite("sub_geo", &RunControl::sub_geo)

		.def_readwrite("obj_function", &RunControl::obj_function)
		.def_readwrite("interp_type",  &RunControl::interp_type)

		//.def_readwrite("rigid_trans",   &RunControl::rigid_trans)
		.def("get_rigid_trans",   get_rigid_trans)
		.def("set_rigid_trans",   set_rigid_trans)
		.def_readwrite("basin_radius",  &RunControl::basin_radius)
		//.def_readwrite("subvol_aspect", &RunControl::subvol_aspect)
		.def("get_subvol_aspect", get_subvol_aspect)
		.def("set_subvol_aspect", set_subvol_aspect)

		.def_readwrite("fine_srch", &RunControl::fine_srch)

		.def("parse_npy" , wparse_npy)
		//.def("get_npy_header_offset" , )
		.def("run_dvc_cmd", run_dvc_cmd)
		;

	bp::enum_<Objfcn_Type>("Objfcn_Type")
		.value("SAD",    SAD)
		.value("SSD",    SSD)
		.value("ZSSD",   ZSSD)
		.value("NSSD",   NSSD)
		.value("ZNSSD",  ZNSSD)
		;
	bp::enum_<Interp_Type>("Interp_Type")
		.value("nearest", nearest)
		.value("trilinear", trilinear)
		.value("tricubic", tricubic)
		;
	bp::enum_<Search_Type>("Search_Type")
		.value("global", global)
		.value("amoeba", amoeba)
		.value("congrad", congrad)
		.value("qnwtdfp", qnwtdfp)
		.value("powells", powells)
		;
	bp::enum_<Subvol_Type>("Subvol_Type")
		.value("cube", cube)
		.value("sphere", sphere)
		;
	bp::enum_<Endian_Type>("Endian_Type")
		.value("little", little)
		.value("big", big)
		;
	bp::enum_<On_Off_Type>("On_Off_Type")
		.value("on", on)
		.value("off", off)
		;
	/*******************************************************************/

	bp::class_<DataCloud>("DataCloud")
		.def("organize_cloud", &DataCloud::organize_cloud)
		.def("n_n_m", &DataCloud::n_n_m)
		.def("n_n_p", &DataCloud::n_n_p)
		.def("n_d_p", &DataCloud::n_d_p)
		.def("loadPointCloudFromNumpy", loadPointCloudFromNumpy)
		.def("getPoint", getPoint)
		;
}

