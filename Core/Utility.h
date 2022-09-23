/*
Copyright 2018 United Kingdom Research and Innovation
Copyright 2018 Oregon State University

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Author(s): Brian Bay,
		   Edoardo Pasca,

*/

#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <vector>

#define PI 3.14159265
#define D2R PI/180.0
#define R2D 180.0/PI

// used during input file reading, ** ought to be an enum
const int input_line_ok =  1;
const int keywd_missing = -1;
const int param_invalid = -2;

// used to communicate search result status, ** ought to be an enum
const int point_good =  0;
const int range_fail = -1;
const int convg_fail = -2;
const int not_search = -3;

// used to define and control input options
/******************************************************************************/
const std::string ok_on_off_line = "on off";
enum On_Off_Type {
	on, off
};
/******************************************************************************/
const std::string ok_endian_line = "little big";
enum Endian_Type {
	little, big
};
/******************************************************************************/
const std::string ok_subvol_geom_line = "cube sphere";
enum Subvol_Type {
	cube, sphere
};
/******************************************************************************/
const std::string ok_srch_mthd_line = "global amoeba congrad qnwtdfp powells";
enum Search_Type {
	global, amoeba, congrad, qnwtdfp, powells
};
/******************************************************************************/
const std::string ok_obj_fcn_line = "sad ssd zssd nssd znssd";
enum Objfcn_Type {
	SAD, SSD, ZSSD, NSSD, ZNSSD
};
/******************************************************************************/
const std::string ok_interp_mthd_line = "nearest trilinear tricubic";
enum Interp_Type {
	nearest, trilinear, tricubic
};
/******************************************************************************/

// used in throw-catch during minimization
class Point_Good {};	// search point located with normal return
class Range_Fail {};	// optimization displaceced to a boundary
class Convg_Fail {};	// max it exceeded in an optimization routine

// used in throw-catch during geometric manipulations
class Bound_Fail {};	// BoundBox not within another, as expected
class Intrp_Fail {};	// Points asked for outside of an interp volume

// used in input operations and fiel checking
class Input_Fail {};	// Reguired input file missing or unreadable

/******************************************************************************/

struct RunControl
{
	// the values read and verified from the input file
	std::string ref_fname;	// the reference image volume
	std::string cor_fname;	// the image volume for correlation
	std::string pts_fname;	// search point cloud data
	std::string out_fname;	// base name for output files
	std::string res_fname;	// dvc analysis results, the displacement file
	std::string sta_fname;	// dvc status file, input echo and run stats

	int vol_bit_depth;
	std::string vol_endian;
	int vol_hdr_lngth;
	int vol_wide;
	int vol_high;
	int vol_tall;

	std::string subvol_geom;
	int subvol_size;
	int subvol_npts;
	
	std::string subvol_thresh;
	double gray_thresh_min;
	double gray_thresh_max;
	double min_vol_fract;

	int disp_max;
	int num_srch_dof;

	unsigned int num_points_to_process;
	
	Objfcn_Type obj_fcn;
	Interp_Type int_typ;
	Subvol_Type sub_geo;
	
	std::string obj_function;
	std::string interp_type;

	std::vector<double> rigid_trans;
	double basin_radius;
	std::vector<double> subvol_aspect;
	std::vector<double> starting_point;

	std::string fine_srch;
};

/******************************************************************************/


#endif
