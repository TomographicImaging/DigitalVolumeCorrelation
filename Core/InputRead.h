/**
# -*- coding: utf-8 -*-
#   This work is part of the Core Imaging Library developed by
#   Visual Analytics and Imaging System Group of the Science Technology
#   Facilities Council, STFC

#   Copyright 2018 CCPi

#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at

#       http://www.apache.org/licenses/LICENSE-2.0

#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

#   Code is derived from code developed by Prof. Brian Bay
*/
#ifndef INPUTREAD_H
#define INPUTREAD_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <limits>
#include <cstdlib>

// adjust Makefile if changes made here
#include "Point.h"
#include "BoundBox.h"
#include "Utility.h"
//

#include "CCPiDefines.h"

// Global variable access
extern double version;
extern int day_rev;
extern std::string month_rev;
extern int year_rev;
	
extern const int input_line_ok;
extern const int keywd_missing;
extern const int param_invalid;

/******************************************************************************/
class CCPI_EXPORT key_word_help
{
public:

	std::string word; // the keyword itself
	std::string exam; // an example (default?) values for the parameter
	std::string reqd; // yes, or the conditions where it is required
	std::string good; // a description of good values for this keyword
	std::string pool; // fio_name, vox_data, sub_vols, opt_mthd, opt_tune
	std::string hint; // a brief description of the keyword
	std::string help; // a detailed description of the keyword

private:
};
/******************************************************************************/
class CCPI_EXPORT InputRead
{
public:

	InputRead();
	~InputRead();

	std::ifstream input_file;

	int input_file_accessible(std::string fname);

	int input_file_read(RunControl *run);

	int read_point_cloud(RunControl *run, std::vector<Point> &search_points, std::vector<int> &search_labels);	

	BoundBox *search_box;
	int search_num_pts;

	// keywords in pool fio_name
	key_word_help kwh_ref_fname;
	key_word_help kwh_cor_fname;
	key_word_help kwh_pts_fname;
	key_word_help kwh_out_fname;

	// keywords in pool vox_data
	key_word_help kwh_vol_bit_depth;
	key_word_help kwh_vol_endian;
	key_word_help kwh_vol_hdr_lngth;
	key_word_help kwh_vol_wide;
	key_word_help kwh_vol_high;
	key_word_help kwh_vol_tall;

	// keywords in pool sub_vols
	key_word_help kwh_subvol_geom;
	key_word_help kwh_subvol_size;
	key_word_help kwh_subvol_npts;

	key_word_help kwh_subvol_thresh;
	key_word_help kwh_gray_thresh_min;
	key_word_help kwh_gray_thresh_max;
	key_word_help kwh_min_vol_fract;

	// keywords in pool opt_mthd
	key_word_help kwh_disp_max;
	key_word_help kwh_num_srch_dof;
	key_word_help kwh_obj_function;
	key_word_help kwh_interp_type;

	// keywords in pool opt_tune
	key_word_help kwh_rigid_trans;
	key_word_help kwh_basin_radius;
	key_word_help kwh_subvol_aspect;


	key_word_help kwh_fine_srch;		// not implemented

	// organized for creation of a manual

	std::vector<key_word_help> manual;

	// data used for value checking

	std::vector<int> ok_vol_bit_depth;
	std::vector<std::string> ok_vol_endian;
	int vol_dim_min, vol_dim_max;
	int vol_hdr_min, vol_hdr_max;
	unsigned long ref_file_length;
	unsigned long cor_file_length;
	unsigned long pts_file_length;

	std::vector<std::string> ok_subvol_geom;
	int subvol_size_min, subvol_size_max;
	int subvol_npts_min, subvol_npts_max;
	std::vector<std::string> ok_subvol_thresh;
	double min_vol_fract_min, min_vol_fract_max;

	std::vector<int> ok_num_srch_dof;
	
	std::vector<std::string> ok_obj_function;
	
	std::vector<std::string> ok_interp_type;

	std::vector<std::string> ok_fine_srch;
	
	double aspect_min;
	double aspect_max;

	std::string limits_to_string(int min, int max);
	std::string limits_to_string(double min, double max);
	std::string limits_to_string(std::vector<std::string> val);
	std::string limits_to_string(std::vector<int> val);
	
	std::vector<std::string> line_to_vect(std::string line);

	// get and parse line functions
	int check_eol(std::ifstream &file, char &eol, std::string &term);
	int get_line_with_keyword(std::string keyword, std::string &keyline, bool req);

	char inp_eol;		// input file eol and line termination
	std::string inp_term;

	// read and check keyword parameters

	int parse_line_old_file(key_word_help kwh, std::string &arg1, unsigned long &bytes, bool req);
	int parse_line_new_file(key_word_help kwh, std::string &arg1, bool req);
	int parse_line_vec_val(key_word_help kwh, std::vector<int> vals, int &arg1, bool req);
	int parse_line_vec_val(key_word_help kwh, std::vector<std::string> vals, std::string &arg1, bool req);
	int parse_line_min_max(key_word_help kwh, int min, int max, int &arg1, bool req);
	int parse_line_min_max(key_word_help kwh, double min, double max, double &arg1, bool req);
	int parse_line_min_max_rel(key_word_help kwh, double min, double max, double &arg1, double &arg2, bool req);	
	int parse_line_dvect(key_word_help kwh, std::vector<double> &vect_lim, std::vector<double> &vect_val, bool req);
	int parse_line_dvect(key_word_help kwh, double min, double max, std::vector<double> &vect_val, bool req);

	// output functions

	int print_manual_intro(std::ofstream &file);
	int print_manual_section(std::ofstream &file, std::string pool);
	int print_manual_output(std::ofstream &file);
	int print_input_example(std::ofstream &file, std::string pool);
	int echo_input(RunControl *run);
	int append_time_date(std::string fname, std::string label, char* dt);
	int append_time_date(std::string fname, std::string label, time_t dt);

	int result_header(std::string fname, int num_params);
	int append_result(std::string fname, int n, Point pt, const int status, double obj_min, std::vector<double> result);

private:

};
/******************************************************************************/
class CCPI_EXPORT DispRead: public InputRead
{
public:

	int read_disp_file_cst_sv(std::string fname, std::vector<int> &label, std::vector<Point> &pos, std::vector<int> &status, std::vector<double> &objmin, std::vector<Point> &dis);

	int read_sort_file_cst_sv(std::string fname, std::vector<std::vector<int>>  &neigh);	// comma, space, or tab seperated variable

	int get_val(std::stringstream &ss, int &val);
	int get_val(std::stringstream &ss, double &val);
	int get_val(std::stringstream &ss, Point &val);

private:

};
/******************************************************************************/

#endif
