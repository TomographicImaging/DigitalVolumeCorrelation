/*
Copyright 2018 United Kingdom Research and Innovation
Copyright 2018 Oregon State University

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Author(s): Brian Bay (OSU)
           Edoardo Pasca (UKRI-STFC)
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
#include <ctime>

// for new point cloud read code
#include <cerrno>
#include <climits>
#include <algorithm>
#include <utility>

// for generalized point could read code
#include <cstddef>
#include <iosfwd>
#include <istream>

// adjust Makefile if changes made here
#include "Point.h"
#include "BoundBox.h"
#include "Utility.h"

#include "CCPiDefines.h"

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
	std::string dflt; // default setting, if one exists, or "none"
	std::string pool; // fio_name, vox_data, sub_vols, opt_mthd, opt_tune
	std::string hint; // a brief description of the keyword
	std::string help; // a detailed description of the keyword

	// --- used only by print_input_form_html() (the HTML input file builder) ---
	// set these next to the other fields, from the same constants the parser uses,
	// so the form checks values exactly as input_file_read() will
	std::string form_type = "text";			// int, real, choice, file_old, file_new, text
	int form_nval = 1;						// number of values on the keyword line
	std::vector<std::string> form_opts;		// drop-down list for form_type "choice"
	bool form_has_min = false, form_has_max = false;
	double form_min = 0.0, form_max = 0.0;	// inclusive range applied to each value
	std::vector<std::string> form_lim_kw;	// |value[i]| <= value of keyword form_lim_kw[i]
	std::string form_cond_kw;				// conditional keywords: required only when
	std::vector<std::string> form_cond_vals;	//   keyword form_cond_kw has one of these values

	void set_form_int(int min, int max) { form_type = "int"; form_has_min = form_has_max = true; form_min = min; form_max = max; }
	void set_form_real(double min, double max, int nval = 1) { form_type = "real"; form_nval = nval; form_has_min = form_has_max = true; form_min = min; form_max = max; }
	void set_form_choice(const std::vector<std::string> &opts) { form_type = "choice"; form_opts = opts; }
	void set_form_choice(const std::vector<int> &opts) { form_type = "choice"; form_opts.clear(); for (int v : opts) form_opts.push_back(std::to_string(v)); }
	void set_form_cond(const std::string &kw, const std::vector<std::string> &vals) { form_cond_kw = kw; form_cond_vals = vals; }

private:
};
/******************************************************************************/
class CCPI_EXPORT InputRead
{
public:

	InputRead();
	~InputRead();

	std::ifstream input_file;

	int input_file_read(RunControl *run);

	// legacy version of point cloud read
	int read_point_cloud(RunControl *run, std::vector<Point> &search_points, std::vector<int> &search_labels);	

	// tab/space/comma version, with capacity for added column and a column header row, and # comment lines
	int read_point_cloud_tsc(RunControl *run, std::vector<Point> &search_points, std::vector<int> &search_labels,
												std::vector<std::vector<double>> &added_columns,
													std::vector<std::string> &added_column_names);

	// generalized point cloud and .disp read functions
	int read_point_cloud_generalized(RunControl *run,
                        std::vector<int>& search_labels,
                        std::vector<Point>& search_points,
                        std::vector<std::vector<double> >& added_columns,
                        std::vector<std::string>& added_column_names);

	int read_disp_file_generalized(std::string fname,
                        std::vector<int>& search_labels,
                        std::vector<Point>& search_points,
                        std::vector<int> &status, 
						std::vector<double> &objmin, 
						std::vector<Point> &dis);					


	int find_flag(std::string flag, int &argc, char *argv[]);
	int find_flag(std::string flag, int &argc, char *argv[], int &val);
	int find_flag(std::string flag, int &argc, char *argv[], double &val);
	int find_flag(size_t pos, size_t len, std::string flag, int &argc, char *argv[]);

	int input_file_accessible(std::string fname);

	BoundBox *search_box;
	int search_num_pts;

	// *** keyword list, grouped by pool

	// keywords in pool fio_name
	key_word_help kwh_ref_fname;
	key_word_help kwh_cor_fname;
	key_word_help kwh_pts_fname;
	key_word_help kwh_out_fname;
	key_word_help kwh_num_points_to_process;	// optional, default (0) is run all points

	// keywords in pool vox_data
	key_word_help kwh_vol_bit_depth;
	key_word_help kwh_vol_endian;		// conditional, only for bit depth = 16
	key_word_help kwh_vol_hdr_lngth;
	key_word_help kwh_vol_wide;
	key_word_help kwh_vol_high;
	key_word_help kwh_vol_tall;

	// keywords in pool sub_vols
	key_word_help kwh_subvol_geom;
	key_word_help kwh_subvol_size;
	key_word_help kwh_subvol_npts;
	key_word_help kwh_subvol_aspect;	// optional

	// keywords in pool opt_mthd
	key_word_help kwh_num_srch_dof;
	key_word_help kwh_obj_function;
	key_word_help kwh_interp_type;
	key_word_help kwh_start_position;
	key_word_help kwh_start_estimate;
	key_word_help kwh_step_max;

	// keywords in pool opt_tune
	key_word_help kwh_cost_tol;		// optional
	key_word_help kwh_step_tol;		// optional
	// key_word_help kwh_grad_tol;	// optional, not currently used
	key_word_help kwh_max_iter;		// optional

	// organized for creation of a manual
	std::vector<key_word_help> manual;

	// *** data used for value checking

	// these are defined in Utility.h as enum's with names stored in a single std::string
	// translated into a vector of individual std::string's in the InputRead constructor
	std::vector<std::string> ok_vol_endian;
	std::vector<std::string> ok_subvol_geom;
	std::vector<std::string> ok_obj_function;
	std::vector<std::string> ok_interp_type;

	// default values are set as the enum integer value, see Utility.h
	const int vol_endian_def = 0;		// array/enum value of little
	const int subvol_geom_def = 0;		// array/enum value of grid
	const int obj_function_def = 3;		// array/enum value of znssd
	const int interp_type_def = 2;		// array/enum value of tri_bspline_3

	unsigned long ref_file_length;
	unsigned long cor_file_length;
	unsigned long pts_file_length;

	const int num_points_to_process_def = 0;	// process all points

	const std::vector<int> ok_bit_depth = {8,16};
	const int vol_bit_depth_def = 8;

	const int vol_hdr_min = 0;
	const int vol_hdr_max = 4096;

	const int vol_dim_min = 10;
	const int vol_dim_max = 8000;

	const int subvol_size_min = 10;
	const int subvol_size_max = 100;
	const int subvol_size_def = 25;

	const int subvol_npts_min = 100;
	const int subvol_npts_max = 50000;
	const int subvol_npts_def = 5000;

	const double subvol_aspect_min = 0.25;
	const double subvol_aspect_max = 4.0;
	const double subvol_aspect_def = 1.0;

	const std::vector<int> ok_srch_dof = {3, 6, 12};
	const int srch_dof_def = 12;

	const int step_max_min = 1;
	const int step_max_max = 15;
	const int step_max_def = 5;

	const double cost_tol_min = 1e-8;
	const double cost_tol_max = 1e-2;
	const double cost_tol_def = 1e-5;

	const double step_tol_min = 1e-6;
	const double step_tol_max = 1e-2;
	const double step_tol_def = 1e-3;

	const double grad_tol_min = 1e-8;
	const double grad_tol_max = 1e-2;
	const double grad_tol_def = 1e-5;

	const int max_iter_min = 1;
	const int max_iter_max = 100;
	const int max_iter_def = 20;

	// *** functions

	// transfer enum list strings into a vector of strings
	std::vector<std::string> line_to_vect(std::string line);

	// number to string conversions for documentation
	std::string limits_to_string(int min, int max);
	std::string limits_to_string(double min, double max);
	std::string limits_to_string(std::vector<std::string> val);
	std::string limits_to_string(std::vector<int> val);
	std::string set_stream_str(double min, double max, double def, std::string notation, int ndp);
	std::string set_stream_str(double min, double max, std::string notation, int ndp);
	std::string set_stream_str(int min, int max, int def);
	std::string set_stream_str(int min, int max);
	std::string set_num_str(double num, std::string notation, int ndp);
	std::string set_num_str(int num);
	std::string set_num_str(int num, std::string comment);

	// read and check keyword parameters
	int parse_line_old_file(key_word_help kwh, std::string &arg1, unsigned long &bytes, bool req);
	int parse_line_new_file(key_word_help kwh, std::string &arg1, bool req);
	int parse_line_vec_val(key_word_help kwh, std::vector<int> vals, int &arg1, bool req);
	int parse_line_vec_val(key_word_help kwh, std::vector<std::string> vals, std::string &arg1, bool req);
	int parse_line_min_max(key_word_help kwh, int min, int max, int &arg1, bool req);
	int parse_line_min_max(key_word_help kwh, unsigned int min, unsigned int max, unsigned int& arg1, bool req);
	int parse_line_min_max(key_word_help kwh, double min, double max, double &arg1, bool req);
	int parse_line_min_max_rel(key_word_help kwh, double min, double max, double &arg1, double &arg2, bool req);	
	int parse_line_dvect(key_word_help kwh, std::vector<double> &vect_lim, std::vector<double> &vect_val, bool req);
	int parse_line_dvect(key_word_help kwh, double min, double max, std::vector<double> &vect_val, bool req);

	// get and parse line functions
	int check_eol(std::ifstream &file, char &eol, std::string &term);
	int get_line_with_keyword(std::string keyword, std::string &keyline, bool req);

	char inp_eol;		// input file eol and line termination
	std::string inp_term;

	// output functions
	int print_input_example(std::ofstream &file, std::string pool);

	// HTML manual (single self-contained file; print to PDF from a browser)
	int print_manual_html(std::ofstream &file);

	// HTML input file builder (single self-contained file; writes a dvc_input text file)
	int print_input_form_html(std::ofstream &file);
	static std::string html_escape(const std::string &s);
	std::string html_link_keywords(const std::string &text, const std::string &self);
	std::string help_to_html(const std::string &help, const std::string &self);
	int print_current_version();

	int echo_input(RunControl *run);
	int append_time_date(std::string fname, std::string label, char* dt);
	int append_time_date(std::string fname, std::string label, time_t dt);

	int result_header(std::string fname, int num_params);
	int append_result(std::string fname, int n, Point pt, const int status, double obj_min, std::vector<double> result);

	// threshold inputs deactivated, option not implemented in code
	/*
	double min_vol_fract_min, min_vol_fract_max;
	key_word_help kwh_subvol_thresh;
	key_word_help kwh_gray_thresh_min;
	key_word_help kwh_gray_thresh_max;
	key_word_help kwh_min_vol_fract;
	std::vector<std::string> ok_subvol_thresh;
	*/

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

// for generalized row/column data file reading
// specific functions (e.g. read_point_cloud_generalized) are built to purpose
// headers and column data types are configured using the ReadOptions struct

namespace tbl {

enum class ColType { Int, Double, Text };

// One column the caller expects in the file.
struct ColumnSpec
{
    std::string name;      // matched against the header (case-insensitive); also the default name
    ColType     type;
    bool        required;  // rows where this value is missing or invalid are skipped

    ColumnSpec(const std::string& n, ColType t, bool req = true)
        : name(n), type(t), required(req) {}
};

enum class HeaderMode
{
    Auto,     // header = last non-data line before the first data row (if any)
    Present,  // first non-comment, non-blank line is always the header
    Absent    // never treat a line as a header
};

struct ReadOptions
{
    std::vector<ColumnSpec> columns;  // the columns you expect, in their usual file order
    bool       read_extra;            // keep columns beyond 'columns' (variable count)
    ColType    extra_type;            // type used for those extra columns
    HeaderMode header;
    bool       match_by_name;         // if a header names every required spec column, locate columns
                                      // by name; optional columns the header lacks are stored as missing
    char       comment;               // lines starting with this char are skipped ('\0' = none)

    ReadOptions()
        : read_extra(false), extra_type(ColType::Double), header(HeaderMode::Auto),
          match_by_name(true), comment('#') {}
};

// Values stored when an optional value is missing or invalid. Text uses "".
const int    kMissingInt    = INT_MIN;
const double kMissingDouble = std::numeric_limits<double>::quiet_NaN();

// One column of data. Only the vector matching 'type' is filled.
struct Column
{
    std::string              name;
    ColType                  type;
    bool                     in_file;  // false if an optional column wasn't in this file's header
    std::vector<int>         ints;
    std::vector<double>      doubles;
    std::vector<std::string> texts;
};

struct Table
{
    std::vector<Column> columns;  // spec columns first (in spec order), then extra columns
    std::size_t rows;
    std::size_t n_spec;           // number of spec columns at the front of 'columns'
    bool        has_header;
    bool        matched_by_name;  // true if columns were located by header name
    int         skipped_rows;     // data-area rows dropped for a missing/invalid required value
    int         bad_values;       // invalid optional values stored as missing
    int         padded_rows;      // rows with fewer extra columns than the widest row

    Table() : rows(0), n_spec(0), has_header(false), matched_by_name(false),
              skipped_rows(0), bad_values(0), padded_rows(0) {}

    std::size_t n_extra() const { return columns.size() - n_spec; }

    // Case-insensitive lookup by name; returns nullptr if there is no such column.
    const Column* find(const std::string& name) const;
};

// Reads 'filename' according to 'opt'. Returns false (with 'error' set) only if
// the file cannot be opened. Check the counters in 'out' for data problems.
bool read_table(const std::string& filename, const ReadOptions& opt,
                Table& out, std::string& error);

// Same, from an already-open stream.
bool read_table(std::istream& is, const ReadOptions& opt, Table& out);

// Lower-level helpers, usable on their own.
bool read_line(std::istream& is, std::string& line);   // accepts \n, \r\n, \r
void split_fields(const std::string& line, std::vector<std::string>& fields);
bool field_to_int(const std::string& s, int& v);       // whole field must be an int
bool field_to_double(const std::string& s, double& v); // whole field must be a finite number

} // namespace tbl

/******************************************************************************/


#endif
