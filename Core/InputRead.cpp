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
#include "InputRead.h"

#include <algorithm>
#include <cctype>

/******************************************************************************/
int InputRead::find_flag(std::string flag, int &argc, char *argv[]) 
{
	// loop through current arg list, if flag found, remove from list and return true

	for (unsigned int i=0; i<argc; i++) {
		std::string argstr(argv[i]);
		if (argstr.compare(flag) == 0) {
			for (unsigned int j=i; j<argc-1; j++) {
				argv[j] = argv[j+1];
			}
			argc -= 1;
			return 1;
		}
	}
	return 0;
}

/******************************************************************************/
int InputRead::find_flag(std::string flag, int &argc, char *argv[], int &val) 
{
	// look for command line flag followed by a single integer argument

	for (unsigned int i=0; i<argc; i++) {			// look through full argv list
		std::string argstr(argv[i]);
		if (argstr.compare(flag) == 0) {			// found the flag
			if (argc > i+1) {						// if there is a next argument try to convert
				try {								// flag and number found
					val = std::stoi(argv[i+1]);
					for (unsigned int j=i; j<argc-1; j++) { // pull both flag and number out of arg list
						argv[j] = argv[j+2];
					}
					argc -= 2;
					return 1;						// return true
				}
				catch (const std::invalid_argument& ia) {	// flag found but next arg not convertable					
					for (unsigned int j=i; j<argc-1; j++) { // pull flag but not the next argument
						argv[j] = argv[j+1];
					}
					argc -= 1;
					std::cout << "argument after " << flag << " not valid" << std::endl;
					return 0;
				}
			}	
			// no next argument on command line, notify and pull flag
			std::cout << "no argument after " << flag << " flag" << std::endl;
			argc -= 1;
		}
	}
	return 0;
}

/******************************************************************************/
int InputRead::find_flag(std::string flag, int &argc, char *argv[], double &val) 
{
	// look for command line flag followed by a single float-double argument

	for (unsigned int i=0; i<argc; i++) {			// look through full argv list
		std::string argstr(argv[i]);
		if (argstr.compare(flag) == 0) {			// found the flag
			if (argc > i+1) {						// if there is a next argument try to convert
				try {								// flag and number found
					val = std::stod(argv[i+1]);
					for (unsigned int j=i; j<argc-1; j++) { // pull both flag and number out of arg list
						argv[j] = argv[j+2];
					}
					argc -= 2;
					return 1;		// return success
				}
				catch (const std::invalid_argument& ia) {	// flag found but next arg not convertable					
					for (unsigned int j=i; j<argc-1; j++) { // pull flag but not the next argument
						argv[j] = argv[j+1];
					}
					argc -= 1;
					std::cout << "argument after " << flag << " not valid" << std::endl;
					return 0;
				}
			}	
			// no next argument on command line, notify and pull flag
			std::cout << "no argument after " << flag << " flag" << std::endl;
			argc -= 1;
		}
	}
	return 0;
}

/******************************************************************************/
int InputRead::find_flag(size_t pos, size_t len, std::string flag, int &argc, char *argv[]) 
{
	// clear extraneous flags

	for (unsigned int i=0; i<argc; i++) {
		std::string argstr(argv[i]);
		if (argstr.compare(pos, len, flag) == 0) {
			std::cout << "-> unused flag " << argv[i] << " found, ignored" << std::endl;
			for (unsigned int j=i; j<argc-1; j++) {
				argv[j] = argv[j+1];
			}
			argc -= 1;
			return 1;
		}
	}
	return 0;
}

/******************************************************************************/
std::vector<std::string> InputRead::line_to_vect(std::string line)
{
	std::vector<std::string> vect;

	std::istringstream io_line;
	std::string str;

	io_line.str(line);
	for (;;) {
		if (io_line.eof()) break;
		io_line >> str;
		vect.push_back(str);
	}

	return vect;
}

/******************************************************************************/
void InputRead::clear_stream_str(std::ostringstream &the_stream, std::string &the_str)
{
	the_stream.str("");
	the_stream.clear();
	the_str.clear();
}
/******************************************************************************/
std::string InputRead::set_stream_str(double min, double max, double def, std::string notation, int ndp)
{
	std::ostringstream a_stream;
	std::string fixed = "fixed";
	std::string scientific = "scientific";

	if (notation  == fixed) {
		a_stream << std::fixed << std::setprecision(ndp) << min
				<< " <-> "
				<< std::fixed << std::setprecision(ndp) << max
				<< ",  default = " << std::fixed << std::setprecision(ndp) << def;	
	}

	if (notation  == scientific) {
		a_stream << std::scientific << std::setprecision(ndp) << min
				<< " <-> "
				<< std::scientific << std::setprecision(ndp) << max
				<< ",  default = " << std::scientific << std::setprecision(ndp) << def;	
	}

    return std::string(a_stream.str());
}
/******************************************************************************/
std::string InputRead::set_num_str(double num, std::string notation, int ndp)
{
	std::ostringstream a_stream;
	std::string fixed = "fixed";
	std::string scientific = "scientific";

	if (notation  == scientific) {a_stream << std::scientific << std::setprecision(ndp) << num;};
	
    return std::string(a_stream.str());
}
/******************************************************************************/
std::string InputRead::set_num_str(int num)
{
	std::ostringstream a_stream;

	a_stream << num;
	
    return std::string(a_stream.str());
}
/******************************************************************************/
std::string InputRead::set_stream_str(int min, int max, int def)
{
	std::ostringstream a_stream;

	a_stream << min << " <-> " << max << ",  default = " << def;

    return std::string(a_stream.str());
}
/******************************************************************************/
InputRead::InputRead()
{
	Point min_pt(0,0,0);
	Point max_pt(0,0,0);
	search_box = new BoundBox(min_pt, max_pt);

	std::string fixed = "fixed";
	std::string scientific = "scientific";

	std::string valid_input; // used to echo valid parameter options from Utility.h for various keywords

	// std::ostringstream allows formatted conversion of numeric variables
	// .str() extracts the string portion which can then be used as a std::string
	// clear_stream_str(a_stream, a_str); before each use
	std::ostringstream a_stream;
	std::string a_str;

	// parameter limits





	ok_vol_bit_depth.push_back(8);
	ok_vol_bit_depth.push_back(16);
	kwh_vol_bit_depth.good = limits_to_string(ok_vol_bit_depth);

	// keyword vol_endian
	ok_vol_endian = line_to_vect(ok_endian_line);
	kwh_vol_endian.good = limits_to_string(ok_vol_endian);

	vol_hdr_min = 0;
	vol_hdr_max = 4096;
	kwh_vol_hdr_lngth.good = limits_to_string(vol_hdr_min, vol_hdr_max);

	vol_dim_min = 0;
	vol_dim_max = 8000;
	kwh_vol_wide.good = limits_to_string(vol_dim_min, vol_dim_max);
	kwh_vol_high.good = limits_to_string(vol_dim_min, vol_dim_max);
	kwh_vol_tall.good = limits_to_string(vol_dim_min, vol_dim_max);

	// keyword subvol_geom
	ok_subvol_geom = line_to_vect(ok_subvol_geom_line);
	kwh_subvol_geom.good = limits_to_string(ok_subvol_geom);

	


	for (int i=0; i<ok_srch_dof.size(); i++) { std::cout << ok_srch_dof[i] << std::endl;}


	
	
	// keyword obj_function
	ok_obj_function = line_to_vect(ok_obj_fcn_line);
	kwh_obj_function.good = limits_to_string(ok_obj_function);

	// keyword interp_type
	ok_interp_type = line_to_vect(ok_interp_mthd_line);
	kwh_interp_type.good = limits_to_string(ok_interp_type);

	


	// fio_name

	kwh_ref_fname.word = "reference_filename";
	kwh_ref_fname.exam = "reference_volume.raw";
	kwh_ref_fname.reqd = "yes";
	kwh_ref_fname.pool = "fio_name";
	kwh_ref_fname.hint = "### single raster file (raw) of same dimensions as the correlate_volume";
	kwh_ref_fname.good = "name of raw image file (in working directory) or a full path";
	kwh_ref_fname.help.assign("   Specify the file containing the reference image volume.\n");
	kwh_ref_fname.help.append("   Format as a single uncompressed raw data file with fixed-length (or no) header.\n");
	kwh_ref_fname.help.append("   e.g. ImageJ Stack File -> Save As -> Raw Data.");
	kwh_ref_fname.help.append("   Place the file in the current working directory or include path information.\n");
	kwh_ref_fname.help.append("   Reference and Correlate files must have same dimensionality (header, wide, high, tall parameters).\n");
	manual.push_back(kwh_ref_fname);

	kwh_cor_fname.word = "correlate_filename";
	kwh_cor_fname.exam = "correlate_volume.raw";
	kwh_cor_fname.reqd = "yes";
	kwh_cor_fname.pool = "fio_name";
	kwh_cor_fname.hint = "### single raster file (raw) of same dimensions as the reference_volume";
	kwh_cor_fname.good = "name of raw image file (in working directory) or a full path";
	kwh_cor_fname.help.assign("   Specify the file containing the correlation image volume.\n");
	kwh_cor_fname.help.append("   Format as a single uncompressed raw data file with fixed-length (or no) header.\n");
	kwh_cor_fname.help.append("   e.g. ImageJ Stack File -> Save As -> Raw Data.\n");
	kwh_cor_fname.help.append("   Place the file in the current working directory or include path information.\n");
	kwh_cor_fname.help.append("   Reference and Correlate files must have same dimensionality (header, wide, high, tall parameters).\n");
	manual.push_back(kwh_cor_fname);

	kwh_pts_fname.word = "point_cloud_filename";
	kwh_pts_fname.exam = "ROI_points_nxyz.txt";
	kwh_pts_fname.reqd = "yes";
	kwh_pts_fname.pool = "fio_name";
	kwh_pts_fname.hint = "### tab delimited text file containing Region of Interest point labels (n) and (xyz) locations";
	kwh_pts_fname.good = "name of nxyz format txt file (in working directory) or a full path";
	kwh_pts_fname.help.assign("   Specify the file containing data for all measurement points in the Region of Interest (ROI).\n");
	kwh_pts_fname.help.append("   The ROI is defined as a cloud of points that fill a geometric region within the reference volume.\n");
	kwh_pts_fname.help.append("   Point cloud size, shape, and density are completely flexible, as long as all points fall within the image volumes.\n");
	kwh_pts_fname.help.append("   Dense point clouds that accurately reflect sample geometry and measurement objectives yield the best results.\n");
	kwh_pts_fname.help.append("   Finite element meshing or other geometry discretization software is very useful for creating point clouds.\n");
	kwh_pts_fname.help.append("   Each line in the tab delimited file contains an integer point label followed by the x,y,z point location, e.g.\n");
	kwh_pts_fname.help.append("      1   300.7   750.2  208.4  \n");
	kwh_pts_fname.help.append("      2   299.3   750.2  209.6  \n");
	kwh_pts_fname.help.append("      etc.  \n");
	kwh_pts_fname.help.append("   Non-integer voxel locations are admitted, with reference volume interpolation used as needed.\n");
	kwh_pts_fname.help.append("   Place the file in the current working directory or include path information.\n");
	manual.push_back(kwh_pts_fname);

	kwh_out_fname.word = "output_filename";
	kwh_out_fname.exam = "myproject_results";
	kwh_out_fname.reqd = "yes";
	kwh_out_fname.pool = "fio_name";
	kwh_out_fname.hint = "### base name for run status (.stat) and displacement data (.disp) output files";
	kwh_out_fname.good = "valid file name (for working directory) or a writeable path";
	kwh_out_fname.help.assign("   Specify a base output file name for results of dvc code execution.\n");
	kwh_out_fname.help.append("   If a filename alone is given, output files are placed in the current working directory.\n");
	kwh_out_fname.help.append("   Alternatively, a full Unix-style path can precede the filename.\n");
	kwh_out_fname.help.append("   The terminal window owner must have write permission for the target directory.\n");
	manual.push_back(kwh_out_fname);

	kwh_num_points_to_process.word = "num_points_to_process";
	kwh_num_points_to_process.exam = "0";
	kwh_num_points_to_process.reqd = "no";
	kwh_num_points_to_process.pool = "fio_name";
	kwh_num_points_to_process.hint = "### Number of points in the point cloud to process";
	kwh_num_points_to_process.good = "between 1 and the number of points in the point cloud file ";
	kwh_num_points_to_process.help.assign("   Defines the maximum number of points in the point cloud to process.\n");
	kwh_num_points_to_process.help.append("   If unset, or set to 0, it will process all points in the point cloud.\n");
	kwh_num_points_to_process.help.append("   Useful for trial runs to evaluate and set input file parameters.\n");
	manual.push_back(kwh_num_points_to_process);

	// vox_data

	kwh_vol_bit_depth.word = "vol_bit_depth";
	kwh_vol_bit_depth.exam = "8";
	kwh_vol_bit_depth.reqd = "yes";
	kwh_vol_bit_depth.pool = "vox_data";
	kwh_vol_bit_depth.hint = "### 8 or 16";
	kwh_vol_bit_depth.help.assign("   Defines the bit depth of both reference and correlate image volumes.\n");
	kwh_vol_bit_depth.help.append("   Assumes unsigned integer voxel representation.\n");
	kwh_vol_bit_depth.help.append("\n");
	manual.push_back(kwh_vol_bit_depth);

	kwh_vol_endian.word = "vol_endian";
	kwh_vol_endian.exam = "little";
	kwh_vol_endian.reqd = "If vol_bit_depth is not 8";
	kwh_vol_endian.pool = "vox_data";
	kwh_vol_endian.hint = "### little or big, ignored if vol_bit_depth is 8";
	kwh_vol_endian.help.assign("   Defines the byte organization for 16-bit image volumes.\n");
	kwh_vol_endian.help.append("   Find the appropriate setting using ImageJ after cross-platform image transfers.\n");
	kwh_vol_endian.help.append("   Big generates a byte swap: Little preserves the byte order.\n");
	kwh_vol_endian.help.append("\n");
	manual.push_back(kwh_vol_endian);

	kwh_vol_hdr_lngth.word = "vol_hdr_lngth";
	kwh_vol_hdr_lngth.exam = "0";
	kwh_vol_hdr_lngth.reqd = "yes";
	kwh_vol_hdr_lngth.pool = "vox_data";
	kwh_vol_hdr_lngth.hint = "### fixed-length header size, may be zero";
	kwh_vol_hdr_lngth.help.assign("   Defines a fixed header length for both reference and correlate image volumes.\n");
	kwh_vol_hdr_lngth.help.append("   This value will often be 0, as is the case for standard raw data files.\n");
	kwh_vol_hdr_lngth.help.append("\n");
	manual.push_back(kwh_vol_hdr_lngth);

	kwh_vol_wide.word = "vol_wide";
	kwh_vol_wide.exam = "4008";
	kwh_vol_wide.reqd = "yes";
	kwh_vol_wide.pool = "vox_data";
	kwh_vol_wide.hint = "### width in pixels of each slice";
	kwh_vol_wide.help.assign("   Defines the width of slices in both reference and correlate image volumes.\n");
	kwh_vol_wide.help.append("   Wide = x for the coordinate system used by the dvc code, origin at the left.\n");
	kwh_vol_wide.help.append("\n");
	manual.push_back(kwh_vol_wide);

	kwh_vol_high.word = "vol_high";
	kwh_vol_high.exam = "2670";
	kwh_vol_high.reqd = "yes";
	kwh_vol_high.pool = "vox_data";
	kwh_vol_high.hint = "### height in pixels of each slice";
	kwh_vol_high.help.assign("   Defines the height of slices in both reference and correlate image volumes.\n");
	kwh_vol_high.help.append("   High = y for the coordinate system used by the dvc code, origin at the top.\n");
	kwh_vol_high.help.append("\n");
	manual.push_back(kwh_vol_high);

	kwh_vol_tall.word = "vol_tall";
	kwh_vol_tall.exam = "4016";
	kwh_vol_tall.reqd = "yes";
	kwh_vol_tall.pool = "vox_data";
	kwh_vol_tall.hint = "### number of slices in the stack";
	kwh_vol_tall.help.assign("   Defines the number of slices in both reference and correlate image volumes.\n");
	kwh_vol_tall.help.append("   Tall = z for the coordinate system used by the dvc code, origin is the first slice.\n");
	kwh_vol_tall.help.append("\n");
	manual.push_back(kwh_vol_tall);

	// sub_vols

	kwh_subvol_geom.word = "subvol_geom";
	kwh_subvol_geom.exam = "cube";
	kwh_subvol_geom.reqd = "yes";
	kwh_subvol_geom.pool = "sub_vols";
	kwh_subvol_geom.hint = "### cube, sphere: geometry of the subvolumes";
	kwh_subvol_geom.help.assign("   Defines geometry of the subvolumes created around each search point.\n");
	kwh_subvol_geom.help.append("   Cubes are the classic, but essentially arbitrary, subvolume geometry.\n");
	kwh_subvol_geom.help.append("   Try spheres if portions of a point cloud are near a surface or interface.\n");
	kwh_subvol_geom.help.append("\n");
	manual.push_back(kwh_subvol_geom);

	kwh_subvol_size.word = "subvol_size";
	kwh_subvol_size.exam = set_num_str(subvol_size_def);
	kwh_subvol_size.reqd = "yes";
	kwh_subvol_size.pool = "sub_vols";
	kwh_subvol_size.hint = "### side length or diameter, in voxels";
	kwh_subvol_size.good = set_stream_str(subvol_size_min, subvol_size_max, subvol_size_def);
	kwh_subvol_size.help.assign("Defines size (in voxels, side length or diameter) of the subvolumes created around each cloud point.\n");
	kwh_subvol_size.help.append("This is a fundamental DVC parameter. Selection can be challenging with the variety of textures encountered in volumetric imaging.\n");
	kwh_subvol_size.help.append("The general guideline is to encompass three distinctly resolved features along each coordinate direction.\n");
	kwh_subvol_size.help.append("Larger subvolumes reduce displacement variability but also reduce spatial resolution of the results.\n");
	kwh_subvol_size.help.append("Smaller subvolumes exhibit more displacement variability but at better spatial resolution.\n");
	kwh_subvol_size.help.append("The companion iDVC code provides tools for evaluating and documenting the selection of subvolume size.\n");
	manual.push_back(kwh_subvol_size);

	kwh_subvol_npts.word = "subvol_npts";
	kwh_subvol_npts.exam = set_num_str(subvol_npts_def);
	kwh_subvol_npts.reqd = "yes";
	kwh_subvol_npts.pool = "sub_vols";
	kwh_subvol_npts.hint = "### number of points to distribute within the subvol";
	kwh_subvol_npts.good = set_stream_str(subvol_npts_min, subvol_npts_max, subvol_npts_def);
	kwh_subvol_npts.help.assign("   Defines the number of points within each subvolume.\n");
	kwh_subvol_npts.help.append("   In this code, subvolume point locations are NOT voxel-centered and the number is INDEPENDENT of subvolume size.\n");
	kwh_subvol_npts.help.append("   Interpolation within the reference image volume is used to establish templates with arbitrary point locations.\n");
	kwh_subvol_npts.help.append("   For cubes a uniform grid of approximately subvol_npts is generated.\n");
	kwh_subvol_npts.help.append("   For spheres subvol_npts are randomly distributed within the subvolume.\n");
	kwh_subvol_npts.help.append("   This parameter has a strong effect on computation time, so be careful.\n");
	manual.push_back(kwh_subvol_npts);

	kwh_subvol_aspect.word = "subvol_aspect";
	kwh_subvol_aspect.exam = "1.0 1.0 1.0";
	kwh_subvol_aspect.reqd = "no";
	kwh_subvol_aspect.pool = "sub_vols";
	kwh_subvol_aspect.hint = "### stretch or contract the subvolume in any coordinate direction between 0.1 and 10.0";
	kwh_subvol_aspect.good = set_stream_str(subvol_aspect_min, subvol_aspect_max, subvol_aspect_def, fixed, 1);
	kwh_subvol_aspect.help.assign("   A standard subvolume is isotropic, with equivalent size in each coordinate direction.\n");
	kwh_subvol_aspect.help.append("   This parameter describes a change in shape of the subvolume to accommodate elongated texture.\n");
	kwh_subvol_aspect.help.append("   It is only useful if the texture direction is consistent, and aligned with a coordinate direction.\n");
	kwh_subvol_aspect.help.append("   The aspect change is specified as three doubles, indicating stretch/contract in the coordinate directions.\n");
	manual.push_back(kwh_subvol_aspect);

	// opt_mthd

	kwh_num_srch_dof.word = "num_srch_dof";
	kwh_num_srch_dof.exam = "12";
	kwh_num_srch_dof.reqd = "yes";
	kwh_num_srch_dof.pool = "opt_mthd";
	kwh_num_srch_dof.hint = "### 3 (translation), 6 (+rotation), or 12 (+strain)";
	kwh_num_srch_dof.good = limits_to_string(ok_srch_dof);
	kwh_num_srch_dof.help.assign("   Defines the degree-of-freedom set for the final stage of the search.\n");
	kwh_num_srch_dof.help.append("   The actual search process introduces degrees-of-freedom in stages up to this value.\n");
	kwh_num_srch_dof.help.append("   Translation only suffices for a quick, preliminary investigation.\n");
	kwh_num_srch_dof.help.append("   Adding rotation will significantly improve displacement accuracy in most cases.\n");
	kwh_num_srch_dof.help.append("   Strain degrees-of-freedom account for deformation of the subvolumes and improve match accuracy.\n");
	kwh_num_srch_dof.help.append("   3  = translation only\n");
	kwh_num_srch_dof.help.append("   6  = translation plus rotation\n");
	kwh_num_srch_dof.help.append("   12 = translation, rotation and strain\n");
	manual.push_back(kwh_num_srch_dof);

	kwh_obj_function.word = "obj_function";
	kwh_obj_function.exam = "znssd";
	kwh_obj_function.reqd = "yes";
	kwh_obj_function.pool = "opt_mthd";
	kwh_obj_function.hint = "### " + ok_obj_fcn_line;
	kwh_obj_function.help.assign("   Standard objective functions of sum-of-squared differences (SSD) form.\n");
	kwh_obj_function.help.append("   Minimizing squared-differences and maximizing cross-correlation are functionally equivalent.\n");
	kwh_obj_function.help.append("   ssd  = standard SSD without normalization, fast but sensitive to brightness/contrast differences.\n");
	kwh_obj_function.help.append("   zssd  = brightness normalized SSD (value not scaled).\n");
	kwh_obj_function.help.append("   nssd  = contrast normalized SSD.\n");
	kwh_obj_function.help.append("   znssd  = brightness and contrast normalized SSD.\n");
	kwh_obj_function.help.append("   Notes on objective function values:\n");
	kwh_obj_function.help.append("      1. Functions nssd and znssd are preferred, as quality of match can be quantified.\n");
	kwh_obj_function.help.append("      2. The natural range of nssd is [0.0 to 2.0], and of znssd is [0.0 to 4.0].\n");
	kwh_obj_function.help.append("      3. Both are scaled for output into the [0.0 to 1.0] range for ease of comparison.\n");
	manual.push_back(kwh_obj_function);

	kwh_interp_type.word = "interp_type";
	kwh_interp_type.exam = "tri_bspline_3";
	kwh_interp_type.reqd = "yes";
	kwh_interp_type.pool = "opt_mthd";
	kwh_interp_type.hint = "### " + ok_interp_mthd_line;
	kwh_interp_type.help.assign("   Defines the interpolation method used during template matching.\n");
	kwh_interp_type.help.append("   Trilinear is fast but imprecise, useful for preliminary runs and evaluating other parameters.\n");
	kwh_interp_type.help.append("   Tricubic is slower but a good choice for general DVC.\n");
	kwh_interp_type.help.append("   Tri_bspline_3 _5 and _7 are standard bspline interpolation of cubic, quintic, and septic orders.\n");
	kwh_interp_type.help.append("   Selection is best evaluated against correlate image volumes with known displacement/strain fields.\n");
	manual.push_back(kwh_interp_type);

	kwh_start_position.word = "start_position";
	kwh_start_position.exam = "300.0 100.0 200.0";
	kwh_start_position.reqd = "yes";
	kwh_start_position.pool = "opt_mthd";
	kwh_start_position.hint = "### x,y,z location near a cloud point with a reliable start_estimate ";
	kwh_start_position.good.assign("limited to the dimensions of the voxel space");
	kwh_start_position.help.assign("This is a very important parameter, and proper setting requires an understanding of the overall DVC process.\n");
	kwh_start_position.help.append("Point cloud processing begins at a single defined point then progresses in order by distance from that point.\n");
	kwh_start_position.help.append("This wavefront approach facilitates the determination of an accurate parameter estimate to initiate each optimizion step.\n");
	kwh_start_position.help.append("But the first point needs its own parameter estimate. That information is provided under the start_estimate keyword.\n");
	kwh_start_position.help.append("Choose a start_position at a location within or in contact with the point cloud.\n");
	kwh_start_position.help.append("Select a position where image data and sample texture are good, and displacement between reference and correlate volumes is small.\n");
	manual.push_back(kwh_start_position);

	kwh_start_estimate.word = "start_estimate";
	kwh_start_estimate.exam = "3.0 1.0 -2.0";
	kwh_start_estimate.reqd = "yes";
	kwh_start_estimate.pool = "opt_mthd";
	kwh_start_estimate.hint = "### x,y,z displacement (u,v,w) in voxels of at the start_position";
	kwh_start_estimate.good.assign("each component typically a few voxels, limited to the dimensions of the voxel space");
	kwh_start_estimate.help.assign("This parameter coordinates closely with the start_position specification.\n");
	kwh_start_estimate.help.append("All cloud points need a good initial estimate at the beginning of the optimization proccess that determines point displacement.\n");
	kwh_start_estimate.help.append("This parameter defines the initial estimate for the very first point in the cloud to seed the wavefront process.\n");
	kwh_start_estimate.help.append("The companion iDVC graphical interface provides automated and manual visualization-guided means of determining the start_estimate.\n");
	kwh_start_estimate.help.append("But the process can be done manually with clever use of software such as ImageJ.\n");
	kwh_start_estimate.help.append("The goal is to determine how much to move the start position to align it in the correlate volume.\n");
	kwh_start_estimate.help.append("The information needed is an estimate, at best within a voxel or two, at worst on the order of the step_max parameter.\n");
	kwh_start_estimate.help.append("The optimization procss refines further from there.\n");
	manual.push_back(kwh_start_estimate);

	kwh_step_max.word = "step_max";
	kwh_step_max.exam = "5";
	kwh_step_max.reqd = "yes";
	kwh_step_max.pool = "opt_mthd";
	kwh_step_max.hint = "### maximum parameter step allowed during optimization, see also step_tol";
	kwh_step_max.good = set_stream_str(step_max_min, step_max_max, step_max_def);
	kwh_step_max.help.assign("Defines the maximum step allowed during optimization from the initial starting estimate.\n");
	kwh_step_max.help.append("This is a very important parameter used for search process control and execution speed management.\n");
	kwh_step_max.help.append("Cloud points process very quickly if the search is limited to a small region beyond a starting estimate.\n");
	kwh_step_max.help.append("Starting estimates are derived from results of successful processing of nearby (neighborhood) points.\n");
	kwh_step_max.help.append("The step_max parameter sets the size of the search region. \n");
	kwh_step_max.help.append("A small value speeds processing and limits access to local optima. \n");
	kwh_step_max.help.append("Range_Fail results for points are an indication of too small a value for step_max.\n");
	kwh_step_max.help.append("Slow processing and erratic results may appear if the value is too large and the starting estimates are poor.\n");
	manual.push_back(kwh_step_max);

	// opt_tune

	kwh_cost_tol.word = "cost_tol";
	kwh_cost_tol.exam = set_num_str(cost_tol_def, scientific, 1);
	kwh_cost_tol.reqd = "no";
	kwh_cost_tol.pool = "opt_tune";
	kwh_cost_tol.hint = "### optional tuning of objective function convergence tolerance";
	kwh_cost_tol.good = set_stream_str(cost_tol_min, cost_tol_max, cost_tol_def, scientific, 0);
	kwh_cost_tol.help.assign("Convergence checks evaluate a normalized change in optimization variables as iterations proceed.\n");
	kwh_cost_tol.help.append("A cloud point search is considered converged if the final objective function (cost) change is below cost_tol.\n");
	kwh_cost_tol.help.append("Smaller tolerance values tune toward precision, larger values tune toward execution speed.\n");
	kwh_cost_tol.help.append("Each convergence check is independent, consider cost_tol in conjunction with step_tol for balanced performance.\n");
	manual.push_back(kwh_cost_tol);

	kwh_step_tol.word = "step_tol";
	kwh_step_tol.exam = set_num_str(step_tol_def, scientific, 1);
	kwh_step_tol.reqd = "no";
	kwh_step_tol.pool = "opt_tune";
	kwh_step_tol.hint = "### optional tuning of parameter vector convergence tolerance";
	kwh_step_tol.good = set_stream_str(step_tol_min, step_tol_max, step_tol_def, scientific, 0);
	kwh_step_tol.help.assign("Convergence checks evaluate a normalized change in optimization variables as iterations proceed.\n");
	kwh_step_tol.help.append("A cloud point search is considered converged if the final parameter vector (step) change is below step_tol.\n");
	kwh_step_tol.help.append("A second control on optimization is step_max which defines the largest overall step allowed during iteration.\n");
	kwh_step_tol.help.append("Smaller tolerance values tune toward precision, larger values tune toward execution speed.\n");
	kwh_step_tol.help.append("Each convergence check is independent, consider cost_tol in conjunction with step_tol for balanced performance.\n");
	manual.push_back(kwh_step_tol);
	

	kwh_max_iter.word = "max_iter";
	kwh_max_iter.exam = set_num_str(max_iter_def);
	kwh_max_iter.reqd = "no";
	kwh_max_iter.pool = "opt_tune";
	kwh_max_iter.hint = "### optional change in the maximum number of optimization iterations";
	kwh_max_iter.good = set_stream_str(max_iter_min, max_iter_max, max_iter_def);
	kwh_max_iter.help.assign("Optimization is limited to a set number of iterations to manage cases of unreasonably slow convergence.\n");
	kwh_max_iter.help.append("In general a well-posed DVC problem with a good starting point will converge in a few iterations.\n");
	kwh_max_iter.help.append("There is no performance benefit from reducing max_iter as convergence checks break early from the update loop.\n");
	kwh_max_iter.help.append("In unusual cases, if very high precision is sought, increasing max_iter while decreasing cost_tol and step_tol may help.\n");
	manual.push_back(kwh_max_iter);


	// threshold inputs deactivated, option not implemented in code

	// keyword subvol_thresh
	/*
	ok_subvol_thresh = line_to_vect(ok_on_off_line);
	kwh_subvol_thresh.good = limits_to_string(ok_subvol_thresh);

	min_vol_fract_min = 0.0;
	min_vol_fract_max = 1.0;
	kwh_min_vol_fract.good = limits_to_string(min_vol_fract_min, min_vol_fract_max);

	kwh_gray_thresh_min.good.assign("0 <= int <= 2^vol_bit_depth, and < gray_thresh_max");
	kwh_gray_thresh_max.good.assign("0 <= int <= 2^vol_bit_depth, and > gray_thresh_min");
	*/

	/*
	kwh_subvol_thresh.word.assign("subvol_thresh");
	kwh_subvol_thresh.exam.assign("off");
	kwh_subvol_thresh.reqd.assign("yes");
	kwh_subvol_thresh.pool.assign("sub_vols");
	kwh_subvol_thresh.hint.assign("### off, on: evaluate subvolumes based on threshold");
	kwh_subvol_thresh.help.assign("   Defines the state of subvolume thresholding to active (on) or inactive (off).\n");
	kwh_subvol_thresh.help.append("   Useful if there is a simple gray level segmentation between foreground and background.\n");
	kwh_subvol_thresh.help.append("   Subvolumes with little foreground content are not searched and flagged on output.\n");
	kwh_subvol_thresh.help.append("\n");
	manual.push_back(kwh_subvol_thresh);

	kwh_gray_thresh_min.word.assign("gray_thresh_min");
	kwh_gray_thresh_min.exam.assign("25");
	kwh_gray_thresh_min.reqd.assign("If subvol_thresh is on");
	kwh_gray_thresh_min.pool.assign("sub_vols");
	kwh_gray_thresh_min.hint.assign("### lower limit of a gray threshold range");
	kwh_gray_thresh_min.help.assign("   Defines the lower limit of a gray scale threshold range.\n");
	kwh_gray_thresh_min.help.append("   Voxels between (min) and (max) are included in the threshold range.\n");
	kwh_gray_thresh_min.help.append("\n");
	manual.push_back(kwh_gray_thresh_min);

	kwh_gray_thresh_max.word.assign("gray_thresh_max");
	kwh_gray_thresh_max.exam.assign("125");
	kwh_gray_thresh_max.reqd.assign("If subvol_thresh is on");
	kwh_gray_thresh_max.pool.assign("sub_vols");
	kwh_gray_thresh_max.hint.assign("### upper limit of a gray threshold range");
	kwh_gray_thresh_max.help.assign("   Defines the upper limit of a gray scale threshold range.\n");
	kwh_gray_thresh_max.help.append("   Voxels between (min) and (max) are included in the threshold range.\n");
	kwh_gray_thresh_max.help.append("\n");
	manual.push_back(kwh_gray_thresh_max);

	kwh_min_vol_fract.word.assign("min_vol_fract");
	kwh_min_vol_fract.exam.assign("0.2");
	kwh_min_vol_fract.reqd.assign("If subvol_thresh is on");
	kwh_min_vol_fract.pool.assign("sub_vols");
	kwh_min_vol_fract.hint.assign("### only search if subvol fraction is greater than");
	kwh_min_vol_fract.help.assign("   Defines a parameter for pre-checking subvolumes for content.\n");
	kwh_min_vol_fract.help.append("   The fraction of subvolume points within the gray_thresh_min/max range is determined.\n");
	kwh_min_vol_fract.help.append("   If below min_vol_fract, the subvolume is likely in a void or in a background region.\n");
	kwh_min_vol_fract.help.append("   A point failing the test is not searched and flagged on output.\n");
	kwh_min_vol_fract.help.append("\n");
	manual.push_back(kwh_min_vol_fract);
	*/


}
/******************************************************************************/
InputRead::~InputRead()
{
	delete search_box;

	input_file.close();
}
/******************************************************************************//******************************************************************************/
int InputRead::input_file_accessible(std::string fname)
{

	input_file.open(fname.c_str());
	if (!input_file.good())
	{
		std::cout << "\nCannot find input file '" << fname << "'\n\n" ;
		return 0;
	}

	return 1;
}
/******************************************************************************/
int InputRead::check_eol(std::ifstream &file, char &eol, std::string &term)
{
	std::string inp_line;
	int count_n = 0;
	int count_r = 0;

	file.clear();
	file.seekg(0, std::ios::beg);
	while (getline(file,inp_line,'\n')) count_n += 1;

	file.clear();
	file.seekg(0, std::ios::beg);
	while (getline(file,inp_line,'\r')) count_r += 1;

	if (count_n >= count_r)
	{
		eol = '\n';
		term = "\n";	// trial and error fix to read problem
	}

	if (count_r > count_n)
	{
		eol = '\r';
		term = "\n";
	}

	return 1;
}
/******************************************************************************/
int InputRead::input_file_read(RunControl *run)
// load the RunControl struct in Utility
// parameter limit values set and checked
// default values for optional parameters also set in this routine
{
	check_eol(input_file, inp_eol, inp_term);

	// const int input_line_ok =  1;
	// const int keywd_missing = -1;
	// const int param_invalid = -2;

	// the parse function returns:
	//  1 = keyword found and parameters good (successful)
	// -1 = keyword not found
	// -2 = parameters bad
	// if( ... != 1) traps any error, if == traps a specific error

	// fio_name

	if(parse_line_old_file(kwh_ref_fname, run->ref_fname, ref_file_length, true) != input_line_ok ) return 0;
	if(parse_line_old_file(kwh_cor_fname, run->cor_fname, cor_file_length, true) != input_line_ok ) return 0;
	if(parse_line_old_file(kwh_pts_fname, run->pts_fname, pts_file_length, true) != input_line_ok ) return 0;
	if(parse_line_new_file(kwh_out_fname, run->out_fname, true) != 1 ) return 0;

	run->res_fname = run->out_fname + ".disp";
	run->sta_fname = run->out_fname + ".stat";

	if (parse_line_min_max(kwh_num_points_to_process, 0, std::numeric_limits<unsigned int>::max(), run->num_points_to_process, true) != input_line_ok) return 0;
	std::cout << "Number of points to process: " << run->num_points_to_process << std::endl;

	// vox_data

	if(parse_line_vec_val(kwh_vol_bit_depth, ok_vol_bit_depth, run->vol_bit_depth, true) != input_line_ok ) return 0;
	if(run->vol_bit_depth != 8)
	{
		if(parse_line_vec_val(kwh_vol_endian, ok_vol_endian, run->vol_endian, true) != input_line_ok ) return 0;
	}

	if(parse_line_min_max(kwh_vol_hdr_lngth, 0, vol_hdr_max, run->vol_hdr_lngth, true) != input_line_ok ) return 0;
	if(parse_line_min_max(kwh_vol_wide, 0, vol_dim_max, run->vol_wide, true) != input_line_ok ) return 0;
	if(parse_line_min_max(kwh_vol_high, 0, vol_dim_max, run->vol_high, true) != input_line_ok ) return 0;
	if(parse_line_min_max(kwh_vol_tall, 0, vol_dim_max, run->vol_tall, true) != input_line_ok ) return 0;

	// sub_vols

	if(parse_line_vec_val(kwh_subvol_geom, ok_subvol_geom, run->subvol_geom, true) != input_line_ok ) return 0;
	for (int i=0; i<ok_subvol_geom.size(); i++)
		if (run->subvol_geom == ok_subvol_geom[i]) run->sub_geo = (Subvol_Type)i;

	if(parse_line_min_max(kwh_subvol_size, subvol_size_min, subvol_size_max, run->subvol_size, true) != input_line_ok ) return 0;

	if(parse_line_min_max(kwh_subvol_npts, subvol_npts_min, subvol_npts_max, run->subvol_npts, true) != input_line_ok ) return 0;

	run->subvol_aspect.resize(3, subvol_aspect_def);
	if(parse_line_dvect(kwh_subvol_aspect, subvol_aspect_min, subvol_aspect_max, run->subvol_aspect, false) == param_invalid) return 0;

	// opt_mthd

	if(parse_line_vec_val(kwh_num_srch_dof, ok_srch_dof, run->num_srch_dof, true) != input_line_ok ) return 0;

	if(parse_line_vec_val(kwh_obj_function, ok_obj_function, run->obj_function, true) != input_line_ok ) return 0;
	for (int i=0; i<ok_obj_function.size(); i++)
		if (run->obj_function == ok_obj_function[i]) run->obj_fcn = (Objfcn_Type)i;
	
	run->bspline = false;
	run->bspline_order = 0;
	if(parse_line_vec_val(kwh_interp_type, ok_interp_type, run->interp_type, true) != input_line_ok ) return 0;
	for (int i=0; i<ok_interp_type.size(); i++) {
		if (run->interp_type == ok_interp_type[i]) {
			run->int_typ = (Interp_Type)i;
			// bspline types are broken into a boolean flag bspline (true) and a numerical bspline_order (3,5,7)
			std::string bspline = "bspline";
			if (ok_interp_type[i].find(bspline) != std::string::npos) {
				char last_char = ok_interp_type[i].back();
				run->bspline_order = last_char - '0'; // this converts single digit ascii chars to the corresponding int
				run->bspline = true;
			}
		}
	}
	
	std::vector<double> vol_limits(3);
	vol_limits[0] = (double)run->vol_wide;
	vol_limits[1] = (double)run->vol_high;
	vol_limits[2] = (double)run->vol_tall;

	run->start_position.resize(3, std::nan(""));
	if (parse_line_dvect(kwh_start_position, vol_limits, run->start_position, true) != input_line_ok) return 0;

	run->start_estimate.resize(3, 0.0);
	if(parse_line_dvect(kwh_start_estimate, vol_limits, run->start_estimate, true) != input_line_ok) return 0;

	if(parse_line_min_max(kwh_step_max, step_max_min, step_max_max, run->step_max, true) != input_line_ok ) return 0;
	
	// opt_tune

	// new code process, set min/max/def in InputRead.h as const
	run->cost_tol = cost_tol_def;
	run->step_tol = step_tol_def;
	run->grad_tol = grad_tol_def;
	run->max_iter = max_iter_def;

	if(parse_line_min_max(kwh_cost_tol, cost_tol_min, cost_tol_max, run->cost_tol, false) == param_invalid) return 0;
	if(parse_line_min_max(kwh_step_tol, step_tol_min, step_tol_max, run->step_tol, false) == param_invalid) return 0;
//	if(parse_line_min_max(kwh_grad_tol, grad_tol_min, grad_tol_max, run->grad_tol, false) == param_invalid) return 0;
	if(parse_line_min_max(kwh_max_iter, max_iter_min, max_iter_max, run->max_iter, false) == param_invalid) return 0;

	// check image volumes
	unsigned long expected_vol_file_size = (unsigned long) run->vol_hdr_lngth 
											+ (unsigned long) run->vol_wide *  
											  (unsigned long) run->vol_high * 
											  (unsigned long) run->vol_tall * 
											  (unsigned long) (run->vol_bit_depth / 8);

	if (ref_file_length != expected_vol_file_size)
	{
		std::cout << std::endl;
		std::cout << "Reference volume file size does not match the volume description." << std::endl;
		std::cout << "Check vol_bit_depth, vol_hdr_lngth, vol_high, vol_wide, and vol_tall." << std::endl;
		std::cout << std::endl;
		return 0;
	}

	if (cor_file_length != expected_vol_file_size)
	{
		std::cout << std::endl;
		std::cout << "Correlate volume file size does not match the volume description." << std::endl;
		std::cout << "Check vol_bit_depth, vol_hdr_lngth, vol_high, vol_wide, and vol_tall." << std::endl;
		std::cout << std::endl;
		return 0;
	}

	// threshold inputs deactivated, option not implemented in code
	/*
	if(parse_line_vec_val(kwh_subvol_thresh, ok_subvol_thresh, run->subvol_thresh, true) != input_line_ok ) return 0;
	if(run->subvol_thresh == "on")
	{
		if(parse_line_min_max(kwh_gray_thresh_min, 0, pow((double)2,(double)run->vol_bit_depth), run->gray_thresh_min, true) != input_line_ok ) return 0;
		if(parse_line_min_max(kwh_gray_thresh_max, run->gray_thresh_min, pow((double)2,(double)run->vol_bit_depth), run->gray_thresh_max, true) != input_line_ok ) return 0;
		if(parse_line_min_max(kwh_min_vol_fract, 0.0, min_vol_fract_max, run->min_vol_fract, true) != input_line_ok ) return 0;
	}
	*/

	return 1;
}/******************************************************************************/
int InputRead::read_point_cloud(RunControl *run, std::vector<Point> &search_points, std::vector<int> &search_labels)
{
	// add check for point outside of voxel volume
	// make changes in DataCloud version as well

	std::ifstream ifs(run->pts_fname.c_str(), std::ifstream::in);

	char eol;
	std::string term;

	check_eol(ifs, eol, term);

	ifs.clear();
	ifs.seekg(0, std::ios::beg);

	int a_label;
	Point a_point(0.0, 0.0, 0.0);

	int ptn = 0;
	double ptx=0.0,pty=0.0,ptz=0.0;
	int count = 0;

	double min_x = std::numeric_limits<double>::max();
	double max_x = std::numeric_limits<double>::min();

	double min_y = std::numeric_limits<double>::max();
	double max_y = std::numeric_limits<double>::min();

	double min_z = std::numeric_limits<double>::max();
	double max_z = std::numeric_limits<double>::min();

	std::string line;
	std::istringstream ss;

	while (getline(ifs, line, eol))
	{
		line += term;
		ss.str(line);

		ss >> ptn;
		if (ss.fail()) {ss.clear(); continue;}
		char c = ss.peek();
		if ((c == '.')||(c == 'E')) {ss.clear(); continue;}

		if (!(ss >> ptx)) {ss.clear(); continue;}
		if (!(ss >> pty)) {ss.clear(); continue;}
		if (!(ss >> ptz)) {ss.clear(); continue;}

		search_labels.push_back(a_label);
		search_labels[count] = ptn;

		search_points.push_back(a_point);
		search_points[count].move_to(ptx,pty,ptz);

		if (ptx < min_x) min_x = ptx;
		if (ptx > max_x) max_x = ptx;

		if (pty < min_y) min_y = pty;
		if (pty > max_y) max_y = pty;

		if (ptz < min_z) min_z = ptz;
		if (ptz > max_z) max_z = ptz;

		count += 1;
	}

	if (count == 0)
	{
		std::cout << std::endl;
		std::cout << "No points were read, the Point Cloud file may be improperly formatted." << std::endl;
		std::cout << "The expected format is plain text, tab (or other 'white space') delimited." << std::endl;
		std::cout << "Header info is OK as long as it does not match the format of a point description." << std::endl;
		std::cout << "Each line of the file should contain an integer point label and x,y,z coordinates, e.g.:" << std::endl;
		std::cout << "5\t10.72\t15.87\t23.45" << std::endl;
		std::cout << std::endl;
		return 0;
	}

	Point min_pt(min_x, min_y, min_z);
	Point max_pt(max_x, max_y, max_z);
	search_box->move_to(min_pt, max_pt);
	search_num_pts = search_points.size();

	return 1;
}
/******************************************************************************/
int DispRead::read_sort_file_cst_sv(std::string fname, std::vector<std::vector<int>>  &neigh) 
{
	// checked with Mac and Windows generated .sort.csv files
	// working without eol checks
	// Windows files have \r at end of line (ss.peek()) but explicit check not needed
	// Windows file did generate an extra line with no points at end, caught with the col_count check

	std::ifstream ifs(fname.c_str(), std::ifstream::in);	// file open checked in calling routine

	ifs.clear();
	ifs.seekg(0, std::ios::beg);

	std::string line;
	int val;

	int line_count = 0;
	while (getline(ifs, line)) {
		std::stringstream ss(line);
		std::vector<int> int_vect = {};

		int col_count = 0;
		while (ss >> val) {
			int_vect.emplace_back(val);
			if(ss.peek() == ',') ss.ignore();
			if(ss.peek() == ' ') ss.ignore();
			if(ss.peek() == '\t') ss.ignore();
			col_count += 1;
		}

		if (col_count > 0) {
			neigh.emplace_back(int_vect);
			line_count += 1;
		}
	}

	// might change return if line_count = 0

	return 1;
}
/******************************************************************************/
int DispRead::get_val(std::stringstream &ss, int &val) {

	if(ss >> val) {
		if(ss.peek() == ',') ss.ignore();
		if(ss.peek() == ' ') ss.ignore();
		if(ss.peek() == '\t') ss.ignore();
		return 1;
	} else {
		return 0;
	}

}
/******************************************************************************/
int DispRead::get_val(std::stringstream &ss, double &val) {

	if(ss >> val) {
		if(ss.peek() == ',') ss.ignore();
		if(ss.peek() == ' ') ss.ignore();
		if(ss.peek() == '\t') ss.ignore();
		return 1;
	} else {
		return 0;
	}

}
/******************************************************************************/
int DispRead::get_val(std::stringstream &ss, Point &val) {

	double val_x, val_y, val_z;

	if ( get_val(ss,val_x) && get_val(ss,val_y) && get_val(ss,val_z) ) {
		val.move_to(val_x,val_y,val_z);
		return 1;
	} else {
		return 0;
	}

}/******************************************************************************/
int DispRead::read_disp_file_cst_sv(std::string fname, std::vector<int> &label, std::vector<Point> &pos, std::vector<int> &status, std::vector<double> &objmin, std::vector<Point> &dis) {

	// .disp file is:	n	x	y	z	status	objmin	u	v	w (int, 3xdouble->point, int, double, 3xdouble->point)
	// ? define as a class and pass around that way?

	std::ifstream ifs(fname.c_str(), std::ifstream::in);	// file open checked in calling routine

	ifs.clear();
	ifs.seekg(0, std::ios::beg);

	std::string line;

	int num_col = 9;		// to check line read success
	int line_count = 0;	
	int loop_count = 0;

	while (getline(ifs, line)) {
		std::stringstream ss(line);

		// use header line to judge whether or not this is a .disp file
		if (loop_count == 0) {
			if(ss.peek() != 'n') {
				std::cout << "-> .disp file lacks proper header, check command line arguments" << std::endl;
				return 0;
			}
		}

		int ival1,ival2;
		double dval;
		Point pval1(0.0, 0.0, 0.0);
		Point pval2(0.0, 0.0, 0.0);

		int col_count = 0;

		if (get_val(ss,ival1)) { // n
			col_count += 1;
		}

		if (get_val(ss,pval1)) { // x,y,z
			col_count += 3;
		}

		if (get_val(ss,ival2)) { // status
			col_count += 1;
		}

		if (get_val(ss,dval)) { // objmin
			col_count += 1;
		} 

		if (get_val(ss,pval2)) { // u,v,w
			col_count += 3;
		}

		if (col_count == num_col) {
			label.emplace_back(ival1);
			pos.emplace_back(pval1);
			status.emplace_back(ival2);
			objmin.emplace_back(dval);
			dis.emplace_back(pval2);
			line_count += 1;
		}

		loop_count += 1;
	}

	return 1;
}
/******************************************************************************/
int InputRead::print_manual_intro(std::ofstream &file)
{
	file << "#####################################################################\n";
	file << "###                                                               ###\n";
	file << "###   A brief manual for the iDVC code      ###\n";
	file << "###                                                               ###\n";
	file << "#####################################################################\n";
	file << "\n";
	file << "(Best viewed in a simple text editor with a fixed-width font and no line wrapping.)\n";
	file << "\n";

	file << "Copyright 2014 Brian K. Bay (computer code and all documentation)\n";
	file << "\n";

	file << "Created:  1 Jan 2014\n";
	file << "Revised:  " << REV_DATE << std::endl;
	file << "version:  " << VERSION << "\n";
	file << "\n";

	file << "The dvc code is written in c++ with portability and simple compilation in mind.\n";
	file << "At present only a single external library (eigen) is used for sparse matrix interpolation calculations.\n";
	file << "Compilation is Makefile controlled. To do a complete rebuild:\n";
	file << "\n";
	file << "   Delete all object (.o) files in /include/objects\n";
	file << "   Enter make from a terminal window in the main distribution directory.\n";
	file << "\n";
	file << "The dvc code executes with the following command line options:\n";
	file << "\n";
	file << "   dvc \t\t\t   // list command line options in the terminal window\n";
	file << "   dvc help\t\t   // send a brief help message to the terminal window\n";
	file << "   dvc example\t\t   // print an example dvc_input file\n";
	file << "   dvc manual\t\t   // print this manual\n";
	file << "   dvc dvc_input\t   // normal code execution\n";
	file << "\n";

	file << "The file dvc_input is the key to running a digital volume correlation analysis.\n";
	file << "It is a simple text file that contains REQUIRED KEYWORDS and PARAMETERS.\n";
	file << "The code parses this file looking for required keywords and appropriate parameter values.\n";
	file << "Feel free to place comments within your input files.\n";
	file << "Any line in a dvc_input file beginning with a # character is ignored.\n";
	file << "The portion of any line following a # character is ignored.\n";
	file << "Some keywords are only required if other keywords have particular values.\n";
	file << "These CONDITIONAL KEYWORDS are ignored if they are left within an input file but are not needed.\n";
	file << "\n";
	file << "The keywords are organized into five groups:\n";
	file << "\n";
	file << "   fio_name \t\t   // names of existing files and files created during a run\n";
	file << "   vox_data \t\t   // description of the voxel data files that are targeted for analysis\n";
	file << "   sub_vols \t\t   // description of the subvolumes created for the template matching process\n";
	file << "   opt_mthd \t\t   // REQUIRED parameters for the default optimization (template matching) process\n";
	file << "   opt_tune \t\t   // OPTIONAL parameters for tuning and refining the template matching process\n";
	file << "\n";

	file << "The keywords are described below, with information organized as:\n";
	file << "\n";
	file << "key_word \t\t   // the keyword exactly as it appears in the input file\n\n";
	file << "      exemplar: \t\t   // a typical dvc_input line for the keyword\n";
	file << "      required: \t\t   // always required (yes), or the conditions when it is required\n";
	file << "      suitable: \t\t   // valid input description, list of suitable values, range and type information\n\n";
	file << "   Further details concerning functionality associated with the keyword and setting of parameters.\n";
	file << "\n";
	file << "\n";


	return 1;
}
/******************************************************************************/
int InputRead::print_manual_section(std::ofstream &file, std::string pool)
{
	file << "#####################################################################\n";
	file << "###                                                               ###\n";
	file << "###   Keywords in group " << pool << "                                  ###\n";
	file << "###                                                               ###\n";
	file << "#####################################################################\n";
	file << "\n";

	for (int i=0; i<manual.size(); i++)
	{
		if (manual[i].pool == pool)
		{
			file << manual[i].word << "\n\n";
			file << "\texample:\t" << manual[i].word << "\t" << manual[i].exam << "\n";
			file << "\trequired:\t" << manual[i].reqd << "\n";
			file << "\tsuitable:\t" << manual[i].good << "\n";
			file << "\n";
			file << manual[i].help;
		}
	}

	return 1;
}
/******************************************************************************/
int InputRead::print_manual_output(std::ofstream &file)
{
	file << "#####################################################################\n";
	file << "###                                                               ###\n";
	file << "###   Output Generated                                            ###\n";
	file << "###                                                               ###\n";
	file << "#####################################################################\n";
	file << "\n";

	file << "During program execution:\n\n";
	file << "   - Information about each point processed is echoed to the command window.\n";
	file << "   - The point identifier appears first, followed by the [x,y,z] location.\n";
	file << "   - The search status appears next.\n";
	file << "      - Point_Good = successful search convergence within the max displacement.\n";
	file << "      - Range_Fail = max displacement exceeded; consider increasing the step_max parameter.\n";
	file << "      - Convg_Fail = maximum iterations exceeded; consider increasing subvol_size &/or npts.\n";
	file << "\n";
	file << "   - The magnitude of the objective function value at the end of the search is listed as obj=.\n";
	file << "      - For obj_function = sad, ssd, and zssd the value is relative, depending on subvolume size and pixel values.\n";
	file << "      - For obj_function = nssd and znssd the value is scaled between 0 and 2, with zero a perfect match.\n";

	file << "   - The point [x,y,z] displacement is listed next for successful searches.\n";
	file << "\n";
	file << "Following program execution:\n\n";
	file << "   - The STATUS file (.stat) contains: \n";
	file << "      - An echo of the input file used to control program execution.\n";
	file << "      - Information about the point cloud, dvc program version, and run date/time.\n";
	file << "      - Search statistics and timing.\n";
	file << "\n";
	file << "   - The DISPLACEMENT file (.disp) contains: \n";
	file << "      - A tab-delimited text file of the dvc results.\n";
	file << "      - A header line appears first identifying columns:\n\n";
	file << "        n x y z status objmin u v w <phi the psi> <exx eyy ezz exy eyz exz>\n\n";
	file << "      - n = the point identifier\n";
	file << "      - x y z = the point location within the reference volume\n";
	file << "      - status = the search outcome: 0 = successful (no error), -1 = Range_Fail, -2 = Convg_Fail\n";
	file << "      - objmin = the objective function magnitude at the end of the search\n";
	file << "      - u v w = the point displacement: [location in target volume] - [location in reference volume]\n";
	file << "      - <phi the psi> = subvolume rotation, if num_srch_dof = 6 or 12\n";
	file << "      - <exx eyy ezz exy eyz exz> = subvolume strain, if num_srch_dof = 12\n";
	file << "\n";
	file << "   - The program does not currently calculate strain, but that is planned for upcoming releases \n";
	file << "      - MATLAB functions griddata, meshgrid, and gradient are useful for strain calculation.\n";
	file << "      - Displacement data imported into finite element analysis codes is also useful for strain calculation.\n";
	file << "\n";


	return 1;
}
/******************************************************************************/
/******************************************************************************/
/*  HTML manual                                                               */
/*                                                                            */
/*  print_manual_html() writes the complete manual as a single, self-contained */
/*  HTML file (no external CSS/JS). It uses the same key_word_help data in    */
/*  the manual vector as the text manual, so keywords only need to be          */
/*  maintained in one place (the InputRead constructor).                      */
/*                                                                            */
/*  For a PDF: open the .html file in a browser and Print -> Save as PDF.      */
/*  Print styling hides the navigation sidebar and avoids splitting keywords. */
/******************************************************************************/
namespace {

struct ManualPool {
	const char *id;
	const char *title;
	const char *desc;
};

// Group order and descriptions for the HTML manual (same order as the text manual).
const ManualPool manual_pools[] = {
	{"fio_name", "File names",          "Names of existing files and files created during a run."},
	{"vox_data", "Voxel data",          "Description of the voxel data files that are targeted for analysis."},
	{"sub_vols", "Subvolumes",          "Description of the subvolumes created for the template matching process."},
	{"opt_mthd", "Optimization method", "REQUIRED parameters for the default optimization (template matching) process."},
	{"opt_tune", "Optimization tuning", "OPTIONAL parameters for tuning and refining the template matching process."}
};
const int num_manual_pools = sizeof(manual_pools) / sizeof(manual_pools[0]);

std::string html_trim(const std::string &s)
{
	const char *ws = " \t\r\n";
	size_t b = s.find_first_not_of(ws);
	if (b == std::string::npos) return "";
	size_t e = s.find_last_not_of(ws);
	return s.substr(b, e - b + 1);
}

size_t html_indent(const std::string &s)
{
	size_t n = 0;
	for (char c : s) {
		if (c == ' ') n += 1;
		else if (c == '\t') n += 4;
		else break;
	}
	return n;
}

bool is_word_char(char c)
{
	return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

// "1. text", "2) text" -> numbered list item
bool is_numbered_item(const std::string &t, std::string &rest)
{
	size_t i = 0;
	while (i < t.size() && std::isdigit(static_cast<unsigned char>(t[i]))) i++;
	if (i == 0 || i >= t.size() || (t[i] != '.' && t[i] != ')')) return false;
	rest = html_trim(t.substr(i + 1));
	return true;
}

// "ssd  = standard SSD ..." or "3  = translation only" -> option definition
bool is_option_line(const std::string &t, std::string &key, std::string &val)
{
	std::istringstream ss(t);
	std::string eq;
	if (!(ss >> key >> eq) || eq != "=") return false;
	std::getline(ss, val);
	val = html_trim(val);
	return true;
}

} // namespace

/******************************************************************************/
std::string InputRead::html_escape(const std::string &s)
{
	std::string r;
	r.reserve(s.size());
	for (char c : s) {
		switch (c) {
			case '&':  r += "&amp;";  break;
			case '<':  r += "&lt;";   break;
			case '>':  r += "&gt;";   break;
			case '"':  r += "&quot;"; break;
			case '\'': r += "&#39;";  break;
			default:   r += c;
		}
	}
	return r;
}

/******************************************************************************/
// Escape text, then turn any keyword name that appears in it into a link to
// that keyword's entry (skipping the keyword currently being described).
std::string InputRead::html_link_keywords(const std::string &text, const std::string &self)
{
	std::string esc = html_escape(text);
	std::string out;
	size_t i = 0;
	while (i < esc.size()) {
		if (is_word_char(esc[i])) {
			size_t j = i;
			while (j < esc.size() && is_word_char(esc[j])) j++;
			std::string tok = esc.substr(i, j - i);
			bool linked = false;
			if (tok != self && tok.find('_') != std::string::npos) {
				for (size_t k = 0; k < manual.size(); k++) {
					if (manual[k].word == tok) {
						out += "<a class=\"kwref\" href=\"#kw-" + tok + "\"><code>" + tok + "</code></a>";
						linked = true;
						break;
					}
				}
			}
			if (!linked) out += tok;
			i = j;
		} else {
			out += esc[i++];
		}
	}
	return out;
}

/******************************************************************************/
// Convert the plain-text help field into HTML. The help strings were written
// for a fixed-width text manual, so layout is inferred from indentation:
//   - lines at the base indent are joined into paragraphs
//   - 2+ consecutive "name = description" lines become an option table
//   - deeper-indented "1. ..." lines become a numbered list
//   - other deeper-indented lines (e.g. data file samples) stay preformatted
std::string InputRead::help_to_html(const std::string &help, const std::string &self)
{
	enum Kind { BLANK, PARA, OPT, NUM, PRE };

	std::vector<std::string> lines;
	{
		std::istringstream ss(help);
		std::string ln;
		while (std::getline(ss, ln)) lines.push_back(ln);
	}

	size_t base = std::string::npos;
	for (const auto &ln : lines)
		if (!html_trim(ln).empty()) base = std::min(base, html_indent(ln));
	if (base == std::string::npos) return "";

	std::vector<Kind> kind(lines.size());
	std::vector<std::string> a(lines.size()), b(lines.size());
	for (size_t i = 0; i < lines.size(); i++) {
		std::string t = html_trim(lines[i]);
		if (t.empty())                              kind[i] = BLANK;
		else if (html_indent(lines[i]) > base)      kind[i] = is_numbered_item(t, a[i]) ? NUM : PRE;
		else if (is_option_line(t, a[i], b[i]))     kind[i] = OPT;
		else                                        kind[i] = PARA;
		if (kind[i] == PARA || kind[i] == PRE) a[i] = t;
		if (kind[i] == PRE) a[i] = lines[i].substr(std::min(lines[i].size(), base));
	}
	// a lone "x = y" line is ordinary prose, not an option list
	for (size_t i = 0; i < lines.size(); ) {
		if (kind[i] != OPT) { i++; continue; }
		size_t j = i;
		while (j < lines.size() && kind[j] == OPT) j++;
		if (j - i < 2)
			for (size_t k = i; k < j; k++) { kind[k] = PARA; a[k] = html_trim(lines[k]); }
		i = j;
	}

	std::ostringstream out;
	for (size_t i = 0; i < lines.size(); ) {
		Kind k = kind[i];
		if (k == BLANK) { i++; continue; }
		size_t j = i;
		while (j < lines.size() && kind[j] == k) j++;

		if (k == PARA) {
			out << "<p>";
			for (size_t n = i; n < j; n++) out << (n > i ? " " : "") << html_link_keywords(a[n], self);
			out << "</p>\n";
		} else if (k == OPT) {
			out << "<table class=\"opts\">\n";
			for (size_t n = i; n < j; n++)
				out << "<tr><td><code>" << html_escape(a[n]) << "</code></td><td>"
				    << html_link_keywords(b[n], self) << "</td></tr>\n";
			out << "</table>\n";
		} else if (k == NUM) {
			out << "<ol>\n";
			for (size_t n = i; n < j; n++) out << "<li>" << html_link_keywords(a[n], self) << "</li>\n";
			out << "</ol>\n";
		} else { // PRE
			out << "<pre class=\"sample\">";
			for (size_t n = i; n < j; n++) out << html_escape(a[n]) << "\n";
			out << "</pre>\n";
		}
		i = j;
	}
	return out.str();
}

/******************************************************************************/
int InputRead::print_manual_html(std::ofstream &file)
{
	std::ostringstream ver, rev;
	ver << VERSION;
	rev << REV_DATE;

	// ---------- head and styles ----------
	file << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
	        "<meta charset=\"utf-8\">\n"
	        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
	        "<title>DVC Manual " << html_escape(ver.str()) << "</title>\n"
	        "<style>\n"
	        ":root{--bg:#fff;--fg:#1d2327;--muted:#5f6b76;--line:#dde3e8;--panel:#f5f7f9;"
	        "--accent:#1f5f99;--req:#1b7a3d;--cond:#a15c00;--opt:#5f6b76;--code:#eef2f5}\n"
	        "@media (prefers-color-scheme:dark){:root{--bg:#15191c;--fg:#e3e8ec;--muted:#9aa6b0;"
	        "--line:#2c343a;--panel:#1c2226;--accent:#7db4e6;--req:#5cc48a;--cond:#e0a454;--opt:#9aa6b0;--code:#232b31}}\n"
	        "*{box-sizing:border-box}\n"
	        "html{scroll-behavior:smooth}\n"
	        "body{margin:0;background:var(--bg);color:var(--fg);"
	        "font:16px/1.55 -apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Helvetica,Arial,sans-serif}\n"
	        "a{color:var(--accent);text-decoration:none}a:hover{text-decoration:underline}\n"
	        "code,pre{font-family:ui-monospace,SFMono-Regular,Menlo,Consolas,monospace;font-size:.9em}\n"
	        "code{background:var(--code);padding:.05em .35em;border-radius:4px}\n"
	        "pre{background:var(--panel);border:1px solid var(--line);border-radius:6px;padding:.8em 1em;overflow-x:auto}\n"
	        "pre code{background:none;padding:0}\n"
	        ".layout{display:flex;max-width:1200px;margin:0 auto}\n"
	        "nav{position:sticky;top:0;align-self:flex-start;width:250px;flex:none;height:100vh;overflow-y:auto;"
	        "padding:1.5em 1em;border-right:1px solid var(--line);font-size:.9em}\n"
	        "nav h2{font-size:1em;margin:0 0 .6em}\n"
	        "nav ul{list-style:none;margin:0;padding:0}\n"
	        "nav li{margin:.15em 0}\n"
	        "nav ul ul{padding-left:.9em;margin:.2em 0 .6em}\n"
	        "nav ul ul a{color:var(--muted);font-family:ui-monospace,Menlo,Consolas,monospace;font-size:.92em}\n"
	        "main{flex:1;min-width:0;padding:1.5em 2.5em 4em}\n"
	        "h1{font-size:2em;margin:.2em 0}\n"
	        "h2{margin-top:2.2em;padding-bottom:.3em;border-bottom:2px solid var(--line)}\n"
	        ".meta{color:var(--muted);margin:0 0 1.5em}\n"
	        "table{border-collapse:collapse}\n"
	        "table.grid{width:100%;margin:1em 0}\n"
	        "table.grid th,table.grid td{text-align:left;vertical-align:top;padding:.4em .7em;border-bottom:1px solid var(--line)}\n"
	        "table.grid th{background:var(--panel)}\n"
	        ".group-desc{color:var(--muted);margin-top:-.4em}\n"
	        ".kw{border:1px solid var(--line);border-radius:8px;padding:1em 1.3em;margin:1.2em 0;break-inside:avoid;scroll-margin-top:1em}\n"
	        ".kw h3{margin:0;font-size:1.15em;display:flex;align-items:center;gap:.6em;flex-wrap:wrap}\n"
	        ".kw h3 code{font-size:1em;background:none;padding:0}\n"
	        ".summary{color:var(--muted);margin:.2em 0 .8em}\n"
	        ".badge{font-size:.72em;font-weight:600;padding:.12em .55em;border-radius:99px;border:1px solid currentColor;"
	        "font-family:-apple-system,'Segoe UI',Roboto,Arial,sans-serif;letter-spacing:.02em}\n"
	        ".b-req{color:var(--req)}.b-cond{color:var(--cond)}.b-opt{color:var(--opt)}\n"
	        "dl.facts{display:grid;grid-template-columns:max-content 1fr;gap:.3em 1em;margin:.6em 0 .9em;"
	        "background:var(--panel);padding:.7em 1em;border-radius:6px}\n"
	        "dl.facts dt{font-weight:600;color:var(--muted)}dl.facts dd{margin:0}\n"
	        "table.opts{margin:.5em 0 1em}\n"
	        "table.opts td{padding:.2em 1em .2em 0;vertical-align:top}\n"
	        "a.kwref code{color:var(--accent)}\n"
	        "@media (max-width:800px){.layout{display:block}nav{position:static;width:auto;height:auto;"
	        "border-right:none;border-bottom:1px solid var(--line)}main{padding:1em}}\n"
	        "@media print{nav{display:none}main{padding:0}body{font-size:11pt}"
	        "h2{break-before:page}h2.nobreak{break-before:auto}a{color:inherit}"
	        ":root{--bg:#fff;--fg:#000;--panel:#f3f3f3;--code:#f0f0f0}}\n"
	        "</style>\n</head>\n<body>\n<div class=\"layout\">\n";

	// ---------- navigation ----------
	file << "<nav>\n<h2>DVC Manual</h2>\n<ul>\n"
	        "<li><a href=\"#intro\">Introduction</a></li>\n"
	        "<li><a href=\"#running\">Running the code</a></li>\n"
	        "<li><a href=\"#input\">The input file</a></li>\n";
	for (int p = 0; p < num_manual_pools; p++) {
		file << "<li><a href=\"#grp-" << manual_pools[p].id << "\">" << manual_pools[p].title << "</a>\n<ul>\n";
		for (size_t i = 0; i < manual.size(); i++)
			if (manual[i].pool == manual_pools[p].id)
				file << "<li><a href=\"#kw-" << html_escape(manual[i].word) << "\">"
				     << html_escape(manual[i].word) << "</a></li>\n";
		file << "</ul></li>\n";
	}
	file << "<li><a href=\"#example\">Example input file</a></li>\n"
	        "<li><a href=\"#output\">Output generated</a></li>\n"
	        "</ul>\n</nav>\n<main>\n";

	// ---------- introduction ----------
	/*file << "<h1 id=\"intro\">A brief manual for the DVC executable</h1>\n"
	        "<p class=\"meta\">Version " << html_escape(ver.str())
	     << " &middot; Revised " << html_escape(rev.str())
	     << " &middot; Created 1 Jan 2014<br>Copyright 2014 Brian K. Bay (computer code and all documentation)</p>\n"
	        "<p>The dvc code is written in C++ with portability and simple compilation in mind. "
	        "At present only a single external library (Eigen) is used for sparse matrix interpolation calculations.</p>\n"
	        "<p>Compilation is Makefile controlled. To do a complete rebuild:</p>\n"
	        "<ol><li>Delete all object (<code>.o</code>) files in <code>/include/objects</code>.</li>\n"
	        "<li>Enter <code>make</code> from a terminal window in the main distribution directory.</li></ol>\n";*/

	file << "<h1 id=\"intro\">A brief manual for the iDVC executable</h1>\n"
	        "<p class=\"meta\">Version " << html_escape(ver.str())
	     << " &middot; Revised " << html_escape(rev.str())
	     << " &middot; Created 1 Jan 2014<br>Copyright 2014 Brian K. Bay (computer code and all documentation)</p>\n"
		 	"<p>"
	        "The iDVC software was inspired by a need for research-level code with access to internal methods and settings. "
			"Aditional objectives were established to facilitate accessibility and portability among research groups: "
			"c++ development environment, low memory overhead, CPU instead of GPU basis, and open-access. "
	        "</p>\n"
		 	"<p>"
	        "An early and continuing area of focus is biological tissues evaluated through <i>in situ</i> x-ray tomography. "
	        "Challenges within that application environment have driven development of the overall algorithmic approach and broadened its aplicability. "
			"Beginning from a highly generalized context has provided flexibility in managing a range of experimental scenarios. "
			"</p>\n"
			"The code at present is strictly local DVC as global constraint models are not always clear for complex, hierarchical hybrid materials. "
			"Test samples are often geometrically complex and benefit from microstructure-based point clouds as opposed to regular grid regions of interest. "
			"Large displacement and strain fields are frequently encountered and inspired a wavefront/neighborhood approach to point cloud processing. "
			"These requirements translated into code that focuses on the basic DVC subvolume tracking process with limited assumptions about sample response. "
	        "</p>\n";





	file << "<h2 id=\"running\" class=\"nobreak\">Running the code</h2>\n"
	        "<table class=\"grid\">\n<tr><th>Command</th><th>Action</th></tr>\n"
	        "<tr><td><code>dvc</code></td><td>List command line options in the terminal window</td></tr>\n"
	        "<tr><td><code>dvc help</code></td><td>Send a brief help message to the terminal window</td></tr>\n"
	        "<tr><td><code>dvc example</code></td><td>Print an example dvc_input file</td></tr>\n"
	        "<tr><td><code>dvc manual</code></td><td>Print this manual</td></tr>\n"
	        "<tr><td><code>dvc dvc_input</code></td><td>Normal code execution</td></tr>\n"
	        "</table>\n";

	file << "<h2 id=\"input\" class=\"nobreak\">The input file</h2>\n"
	        "<p>The file <code>dvc_input</code> is the key to running a digital volume correlation analysis. "
	        "It is a simple text file that contains <strong>required keywords</strong> and <strong>parameters</strong>. "
	        "The code parses this file looking for required keywords and appropriate parameter values.</p>\n"
	        "<p>Feel free to place comments within your input files. Any line beginning with a <code>#</code> "
	        "character is ignored, as is the portion of any line following a <code>#</code>.</p>\n"
	        "<p>Some keywords are only required if other keywords have particular values. "
	        "These <strong>conditional keywords</strong> are ignored if they are left within an input file but are not needed.</p>\n"
	        "<p>The keywords are organized into " << num_manual_pools << " groups:</p>\n"
	        "<table class=\"grid\">\n<tr><th>Group</th><th>Contents</th></tr>\n";
	for (int p = 0; p < num_manual_pools; p++)
		file << "<tr><td><a href=\"#grp-" << manual_pools[p].id << "\"><code>" << manual_pools[p].id
		     << "</code></a></td><td>" << manual_pools[p].desc << "</td></tr>\n";
	file << "</table>\n"
	        "<p>Each keyword entry below lists:</p>\n"
	        "<dl class=\"facts\">"
	        "<dt>Example</dt><dd>a typical dvc_input line for the keyword</dd>"
	        "<dt>Required</dt><dd>always required, optional, or the conditions when it is required</dd>"
	        "<dt>Suitable</dt><dd>valid input description, list of suitable values, range and type information</dd>"
	        "</dl>\n<p>followed by further details on the functionality associated with the keyword and how to set its parameters.</p>\n";

	// ---------- keyword groups ----------
	for (int p = 0; p < num_manual_pools; p++) {
		file << "<h2 id=\"grp-" << manual_pools[p].id << "\">" << manual_pools[p].title
		     << " <small><code>" << manual_pools[p].id << "</code></small></h2>\n"
		     << "<p class=\"group-desc\">" << manual_pools[p].desc << "</p>\n";

		for (size_t i = 0; i < manual.size(); i++) {
			const key_word_help &k = manual[i];
			if (k.pool != manual_pools[p].id) continue;

			std::string w = html_escape(k.word);

			// requirement badge
			std::string badge;
			if (k.reqd == "yes")     badge = "<span class=\"badge b-req\">required</span>";
			else if (k.reqd == "no") badge = "<span class=\"badge b-opt\">optional</span>";
			else                     badge = "<span class=\"badge b-cond\">conditional</span>";

			// one-line summary from the hint: first line, without the "###" and "keyword:" prefixes
			std::string summary = k.hint.substr(0, k.hint.find('\n'));
			summary = html_trim(summary.substr(std::min(summary.size(), summary.find_first_not_of("# "))));
			if (summary.compare(0, k.word.size() + 1, k.word + ":") == 0)
				summary = html_trim(summary.substr(k.word.size() + 1));

			std::string reqd = (k.reqd == "yes") ? "Yes" : (k.reqd == "no") ? "No" : k.reqd;

			file << "<section class=\"kw\" id=\"kw-" << w << "\">\n"
			     << "<h3><code>" << w << "</code>" << badge << "</h3>\n";
			if (!summary.empty())
				file << "<p class=\"summary\">" << html_link_keywords(summary, k.word) << "</p>\n";
			file << "<dl class=\"facts\">\n"
			     << "<dt>Example</dt><dd><code>" << w << "&nbsp;&nbsp;" << html_escape(html_trim(k.exam)) << "</code></dd>\n"
			     << "<dt>Required</dt><dd>" << html_link_keywords(reqd, k.word) << "</dd>\n"
			     << "<dt>Suitable</dt><dd>" << html_link_keywords(k.good, k.word) << "</dd>\n"
			     << "</dl>\n"
			     << help_to_html(k.help, k.word)
			     << "</section>\n";
		}
	}

	// ---------- example input file (same content as "dvc example") ----------
	file << "<h2 id=\"example\">Example input file</h2>\n"
	        "<p>A complete <code>dvc_input</code> file. Optional and conditional keywords are commented out with <code>#</code>.</p>\n<pre><code>";
	for (int p = 0; p < num_manual_pools; p++) {
		file << "###\n###  Keywords in group " << manual_pools[p].id << "\n###\n\n";
		for (size_t i = 0; i < manual.size(); i++) {
			const key_word_help &k = manual[i];
			if (k.pool != manual_pools[p].id) continue;
			std::string hint = k.hint.substr(0, k.hint.find('\n'));
			file << (k.reqd != "yes" ? "# " : "") << html_escape(k.word) << "\t"
			     << html_escape(html_trim(k.exam)) << "\t" << html_escape(hint) << "\n";
		}
		file << "\n";
	}
	file << "</code></pre>\n";

	// ---------- output ----------
	file << "<h2 id=\"output\">Output generated</h2>\n"
	        "<h3>During program execution</h3>\n<ul>\n"
	        "<li>Information about each point processed is echoed to the command window.</li>\n"
	        "<li>The point identifier appears first, followed by the [x,y,z] location.</li>\n"
	        "<li>The search status appears next:\n<table class=\"opts\">\n"
	        "<tr><td><code>Point_Good</code></td><td>successful search convergence within the max displacement</td></tr>\n"
	        "<tr><td><code>Range_Fail</code></td><td>max displacement exceeded; consider increasing the "
	        "<a class=\"kwref\" href=\"#kw-step_max\"><code>step_max</code></a> parameter</td></tr>\n"
	        "<tr><td><code>Convg_Fail</code></td><td>maximum iterations exceeded; consider increasing "
	        "<a class=\"kwref\" href=\"#kw-subvol_size\"><code>subvol_size</code></a> and/or "
	        "<a class=\"kwref\" href=\"#kw-subvol_npts\"><code>subvol_npts</code></a></td></tr>\n"
	        "</table></li>\n"
	        "<li>The magnitude of the objective function value at the end of the search is listed as <code>obj=</code>.\n<ul>\n"
	        "<li>For <code>obj_function</code> = sad, ssd, and zssd the value is relative, depending on subvolume size and pixel values.</li>\n"
	        "<li>For <code>obj_function</code> = nssd and znssd the value is scaled between 0 and 2, with zero a perfect match.</li>\n"
	        "</ul></li>\n"
	        "<li>The point [x,y,z] displacement is listed next for successful searches.</li>\n</ul>\n";

	file << "<h3>Following program execution</h3>\n"
	        "<p>The <strong>status file</strong> (<code>.stat</code>) contains:</p>\n<ul>\n"
	        "<li>An echo of the input file used to control program execution.</li>\n"
	        "<li>Information about the point cloud, dvc program version, and run date/time.</li>\n"
	        "<li>Search statistics and timing.</li>\n</ul>\n"
	        "<p>The <strong>displacement file</strong> (<code>.disp</code>) is a tab-delimited text file of the dvc results. "
	        "A header line appears first identifying columns:</p>\n"
	        "<pre><code>n x y z status objmin u v w &lt;phi the psi&gt; &lt;exx eyy ezz exy eyz exz&gt;</code></pre>\n"
	        "<table class=\"grid\">\n<tr><th>Column</th><th>Meaning</th></tr>\n"
	        "<tr><td><code>n</code></td><td>the point identifier</td></tr>\n"
	        "<tr><td><code>x y z</code></td><td>the point location within the reference volume</td></tr>\n"
	        "<tr><td><code>status</code></td><td>the search outcome: 0 = successful (no error), -1 = Range_Fail, -2 = Convg_Fail</td></tr>\n"
	        "<tr><td><code>objmin</code></td><td>the objective function magnitude at the end of the search</td></tr>\n"
	        "<tr><td><code>u v w</code></td><td>the point displacement: [location in target volume] &minus; [location in reference volume]</td></tr>\n"
	        "<tr><td><code>phi the psi</code></td><td>subvolume rotation, if <a class=\"kwref\" href=\"#kw-num_srch_dof\"><code>num_srch_dof</code></a> = 6 or 12</td></tr>\n"
	        "<tr><td><code>exx eyy ezz exy eyz exz</code></td><td>subvolume strain, if <a class=\"kwref\" href=\"#kw-num_srch_dof\"><code>num_srch_dof</code></a> = 12</td></tr>\n"
	        "</table>\n"
	        "<p>The program does not currently calculate strain, but that is planned for upcoming releases.</p>\n<ul>\n"
	        "<li>MATLAB functions <code>griddata</code>, <code>meshgrid</code>, and <code>gradient</code> are useful for strain calculation.</li>\n"
	        "<li>Displacement data imported into finite element analysis codes is also useful for strain calculation.</li>\n</ul>\n";

	file << "</main>\n</div>\n</body>\n</html>\n";

	return 1;
}
int InputRead::print_input_example(std::ofstream &file, std::string pool)
{
	file << "###\n###\t Keywords in group " << pool << "\n###\n\n";

	for (int i=0; i<manual.size(); i++)
	{
		if (manual[i].pool == pool)
		{
			if (manual[i].reqd != "yes")
				file << "# ";

			file << manual[i].word << "\t" << manual[i].exam << "\t" << manual[i].hint << "\n";
		}
	}

	file << "\n";

	return 1;
}
/******************************************************************************/
int InputRead::print_current_version(){
	std::cout << "CCPi Digital Volume Correlation version: " << VERSION << std::endl;
	return 0;
}

/******************************************************************************/
int InputRead::echo_input(RunControl *run)
{
	std::ofstream sta_file(run->sta_fname.c_str(), std::ios_base::out);

	sta_file << "\n";
	sta_file << "### echo of the input file for this run";
	sta_file << "\n\n";

	// fio_name

	sta_file << kwh_ref_fname.word << "\t" << run->ref_fname << "\n";
	sta_file << kwh_cor_fname.word << "\t" << run->cor_fname << "\n";
	sta_file << kwh_pts_fname.word << "\t" << run->pts_fname << "\n";
	sta_file << kwh_out_fname.word << "\t" << run->out_fname << "\n";
	sta_file << "\n";

	// vox_data

	sta_file << kwh_vol_bit_depth.word << "\t" << run->vol_bit_depth << "\n";
	if (run->vol_bit_depth != 8)
	{
		sta_file << kwh_vol_endian.word << "\t" << run->vol_endian << "\n";
	}
	sta_file << kwh_vol_hdr_lngth.word << "\t" << run->vol_hdr_lngth << "\n";
	sta_file << kwh_vol_wide.word << "\t" << run->vol_wide << "\n";
	sta_file << kwh_vol_high.word << "\t" << run->vol_high << "\n";
	sta_file << kwh_vol_tall.word << "\t" << run->vol_tall << "\n";
	sta_file << "\n";

	// sub_vols

	sta_file << kwh_subvol_geom.word << "\t" << run->subvol_geom << "\n";
	sta_file << kwh_subvol_size.word << "\t" << run->subvol_size << "\n";
	sta_file << kwh_subvol_npts.word << "\t" << run->subvol_npts << "\n";
	sta_file << "\n";

	/*
	sta_file << kwh_subvol_thresh.word << "\t" << run->subvol_thresh  << "\n";
	if(run->subvol_thresh == "on")
	{
		sta_file << kwh_gray_thresh_min.word << "\t" << run->gray_thresh_min  << "\n";
		sta_file << kwh_gray_thresh_max.word << "\t" << run->gray_thresh_max  << "\n";
		sta_file << kwh_min_vol_fract.word << "\t" << run->min_vol_fract << "\n";
	}
	sta_file << "\n";
	*/

	// opt_mthd

	sta_file << kwh_step_max.word << "\t"<< run->step_max << "\n";
	sta_file << kwh_num_srch_dof.word << "\t"<< run->num_srch_dof << "\n";
	sta_file << kwh_obj_function.word << "\t"<< run->obj_function << "\n";
	sta_file << kwh_interp_type.word << "\t"<< run->interp_type << "\n";
	sta_file << "\n";

	// opt_tune

	sta_file << kwh_start_estimate.word << "\t" << run->start_estimate[0] << "\t" << run->start_estimate[1] << "\t" << run->start_estimate[2] << "\n";
//	sta_file << kwh_basin_radius.word << "\t" << run->basin_radius << "\n";
	sta_file << kwh_subvol_aspect.word << "\t" << run->subvol_aspect[0] << "\t" << run->subvol_aspect[1] << "\t" << run->subvol_aspect[2] << "\n";

	// point cloud and version information

	sta_file << "\n";
	sta_file << "### end of input file echo";
	sta_file << "\n";

	sta_file << "\n" << "Point Cloud contains " << search_num_pts << " points\n\n";
	sta_file << "\t" << "bounding box min = [";
	sta_file << search_box->min().x() << " " << search_box->min().y() << " " << search_box->min().z() << "]\n";
	sta_file << "\t" << "bounding box max = [";
	sta_file << search_box->max().x() << " " << search_box->max().y() << " " << search_box->max().z() << "]\n";

	sta_file << "\n" << "running under dvc code version: " << VERSION << "\n\n";

	return 1;
}
/******************************************************************************/
int InputRead::append_time_date(std::string fname, std::string label, char* dt)
{
	std::ofstream sta_file(fname.c_str(), std::ios_base::out | std::ios_base::app);

	sta_file << label << dt << "\n";

	return 1;
}
int InputRead::append_time_date(std::string fname, std::string label, time_t dt)
{
	std::ofstream sta_file(fname.c_str(), std::ios_base::out | std::ios_base::app);

	//doesn't compile in gcc version < 5
	//std::stringstream ss;
	//ss << std::put_time(std::localtime(&dt), "%Y-%m-%d %X");
	//sta_file << label << ss.str() << "\n";

	char ss[30];
	std::strftime(ss, 30*sizeof(char), "%Y-%m-%d %H:%M:%S", std::localtime(&dt));
	sta_file << label << ss << "\n";

	return 1;
}
/******************************************************************************/
int InputRead::result_header(std::string fname, int num_params)
{
	//(u,v,w,phi,the,psi,exx,eyy,ezz,exy,eyz,exz), angles in rad, nondim strain

	std::ofstream res_file(fname.c_str(), std::ios_base::out);

	res_file << "n";

	res_file << "\t" << "x" << "\t" << "y" << "\t" << "z";

	res_file << "\t" << "status";

	res_file << "\t" << "objmin";

	res_file << "\t" << "u" << "\t" << "v" << "\t" << "w";

	// changed .disp output to just include displacements
	/*
	if (num_params > 3) res_file << "\t" << "phi" << "\t" << "the" << "\t" << "psi";

	if (num_params > 6)
	{
		res_file << "\t" << "exx" << "\t" << "eyy" << "\t" << "ezz";
		res_file << "\t" << "exy" << "\t" << "eyz" << "\t" << "exz";
	}
	*/

	res_file << "\n";

	return 1;
}
/******************************************************************************/
int InputRead::append_result(std::string fname, int n, Point pt, const int status, double obj_min, std::vector<double> result)
{
	std::ofstream res_file(fname.c_str(), std::ios_base::out | std::ios_base::app);

	res_file << std::fixed << std::setprecision(0);
	res_file << n;

	res_file << std::fixed << std::setprecision(3);
	res_file << "\t" << pt.x() << "\t" << pt.y() << "\t" << pt.z();

	res_file << std::fixed << std::setprecision(0);
	res_file << "\t" << status;

	res_file << std::fixed << std::setprecision(6);
	res_file << "\t" << obj_min;

	res_file << std::fixed << std::setprecision(6);
	// this prints just the dispalcements
	for (int i=0; i<3; i++) {
		res_file << "\t" << result[i];
	} 
	
	// this prints full search params, disp, rotation, strain if used
	/*
	for (int i=0; i<result.size(); i++)
	{
		res_file << "\t" << result[i];
	} 
	*/

	res_file << "\n";

	return 1;
}
/******************************************************************************/
std::string InputRead::limits_to_string(int min, int max)
{
	std::string std_str;
	std::ostringstream con_str;
	con_str << min << " <= int <= " << max;
	std_str = con_str.str();

	return std_str;
}
/******************************************************************************/
std::string InputRead::limits_to_string(double min, double max)
{
	std::string std_str;
	std::ostringstream con_str;
	con_str << std::fixed << min << " <= double <= " << max;
	std_str = con_str.str();

	return std_str;
}
/******************************************************************************/
std::string InputRead::limits_to_string(std::vector<std::string> val)
{
	std::string std_str;
	std::ostringstream con_str;

	for (int i=0; i<val.size(); i++)
	{
		con_str << val[i];
		if (i<val.size()-1) con_str << ", ";
	}

	std_str = con_str.str();

	return std_str;
}
/******************************************************************************/
std::string InputRead::limits_to_string(std::vector<int> val)
{
	std::string std_str;
	std::ostringstream con_str;

	for (int i=0; i<val.size(); i++)
	{
		con_str << val[i];
		if (i<val.size()-1) con_str << ", ";
	}

	std_str = con_str.str();

	return std_str;
}
/******************************************************************************/
int InputRead::get_line_with_keyword(std::string keyword, std::string &keyline, bool req)
// with the input file already open, look for line containing keyword
// ignore any input file lines starting with a # character, indicating comment
{
	std::string inp_line;		// getline loads into here

	input_file.clear();
	input_file.seekg(0, std::ios::beg);

	while (getline(input_file, inp_line, inp_eol))
	{
		inp_line += inp_term;

		if (inp_line[0]!='#')	// ignore any lines that start with this character
		{
			if (inp_line.find(keyword)!=std::string::npos)	// passes if in_line contains keyword
			{
				keyline = inp_line;
				return 1;
			}
		}

	}

	if (req) std::cout << "\n\nInput Error: keyword '" << keyword << "' not found in input file.\n\n";

	return 0;
}

/******************************************************************************/
int InputRead::parse_line_old_file(key_word_help kwh, std::string &arg1, unsigned long &bytes, bool req)
// a single string on the input line identifying an existing file
// check that file exists and can be opened
{
	std::string keyline;

	if(!get_line_with_keyword(kwh.word, keyline, req))
		return keywd_missing;

	std::istringstream io_keyline;
	std::string word;
	std::string str1;

	io_keyline.str(keyline);

	while (word == "")
		getline(io_keyline, word, '\t');

	while (str1 == "")
		getline(io_keyline, str1, '\t');

	std::ifstream old_file(str1.c_str());

	if (old_file.good())
	{
		arg1 = str1;
		old_file.seekg(0, old_file.end);
		bytes = old_file.tellg();
		old_file.close();
		return 1;
	}

	std::cout << std::endl << "Input Error: " << kwh.word << " cannot find file: " << str1 << std::endl;
	std::cout << kwh.hint << std::endl << std::endl;

	return param_invalid;
}
/******************************************************************************/
int InputRead::parse_line_new_file(key_word_help kwh, std::string &arg1, bool req)
// a single string on the input line identifying a new file
// check that a file can be created at the location indicated
{
	std::string keyline;

	if(!get_line_with_keyword(kwh.word, keyline, req))
		return keywd_missing;

	std::istringstream io_keyline;
	std::string word;
	std::string str1;

	io_keyline.str(keyline);

	while (word == "")
		getline(io_keyline, word, '\t');

	while(str1=="")
		getline(io_keyline, str1, '\t');

	std::ofstream new_file(str1.c_str());

	if (new_file.is_open())
	{
		arg1 = str1;
		new_file.close();
		remove(str1.c_str());
		return 1;
	}

	std::cout << std::endl << "Input Error: " << kwh.word << " contains an invalid parameter." << std::endl;
	std::cout << kwh.hint << std::endl << std::endl;

	return param_invalid;
}
/******************************************************************************/
int InputRead::parse_line_vec_val(key_word_help kwh, std::vector<int> vals, int &arg1, bool req)
// compare int argument against a vector of possible int values
{
	std::string keyline;

	if(!get_line_with_keyword(kwh.word, keyline, req))
		return keywd_missing;

	std::istringstream io_keyline;
	std::string word;
	double int1;

	io_keyline.str(keyline);

	io_keyline >> word >> int1;

	for (int i=0; i<vals.size(); i++)
	{
		if (int1 == vals[i])
		{
			arg1 = int1;
			return 1;
		}
	}

	std::cout << std::endl << "Input Error: " << kwh.word << " contains an invalid parameter." << std::endl;
	std::cout << kwh.hint << std::endl << std::endl;

	return param_invalid;
}
/******************************************************************************/
int InputRead::parse_line_vec_val(key_word_help kwh, std::vector<std::string> vals, std::string &arg1, bool req)
// compare string argument against a vector of possible string values
{
	std::string keyline;

	if(!get_line_with_keyword(kwh.word, keyline, req))
		return keywd_missing;

	std::istringstream io_keyline;
	std::string word;
	std::string str1;

	io_keyline.str(keyline);

	io_keyline >> word >> str1;

	for (int i=0; i<vals.size(); i++)
	{
		if (str1 == vals[i])
		{
			arg1 = str1;
			return 1;
		}
	}

	std::cout << std::endl << "Input Error: " << kwh.word << " contains an invalid parameter." << std::endl;
	std::cout << kwh.hint << std::endl << std::endl;

	return param_invalid;
}
/******************************************************************************/
int InputRead::parse_line_min_max(key_word_help kwh, int min, int max, int &arg1, bool req)
// check that int argument is between min and max inclusive
{
	std::string keyline;

	if(!get_line_with_keyword(kwh.word, keyline, req))
		return keywd_missing;

	std::istringstream io_keyline;
	std::string word;
	int int1;

	io_keyline.str(keyline);

	io_keyline >> word >> int1;

	if ((int1 >= min) && (int1 <= max))
	{
		arg1 = int1;
		return 1;
	}

	std::cout << std::endl << "Input Error: " << kwh.word << " contains an invalid parameter." << std::endl;
	std::cout << kwh.hint << std::endl << std::endl;

	return param_invalid;
}
/******************************************************************************/
int InputRead::parse_line_min_max(key_word_help kwh, unsigned int min, unsigned int max, unsigned int& arg1, bool req)
// check that int argument is between min and max inclusive
{
	std::string keyline;

	if (!get_line_with_keyword(kwh.word, keyline, req))
		return keywd_missing;

	std::istringstream io_keyline;
	std::string word;
	unsigned int int1;

	io_keyline.str(keyline);

	io_keyline >> word >> int1;

	if ((int1 >= min) && (int1 <= max))
	{
		arg1 = int1;
		return 1;
	}

	std::cout << std::endl << "Input Error: " << kwh.word << " contains an invalid parameter." << std::endl;
	std::cout << kwh.hint << std::endl << std::endl;

	return param_invalid;
}

/******************************************************************************/
int InputRead::parse_line_min_max(key_word_help kwh, double min, double max, double &arg1, bool req)
// check that int argument is between min and max inclusive
{
	std::string keyline;

	if(!get_line_with_keyword(kwh.word, keyline, req))
		return keywd_missing;

	std::istringstream io_keyline;
	std::string word;
	double dbl1;

	io_keyline.str(keyline);

	io_keyline >> word >> dbl1;

	if ((dbl1 >= min) && (dbl1 <= max))
	{
		arg1 = dbl1;
		return 1;
	}

	std::cout << std::endl << "Input Error: " << kwh.word << " contains an invalid parameter." << std::endl;
	std::cout << kwh.hint << std::endl << std::endl;

	return param_invalid;
}
/******************************************************************************/
int InputRead::parse_line_min_max_rel(key_word_help kwh, double min, double max, double &arg1, double &arg2, bool req)
// check that both doubles fall between min and max inclusive
// check that the second double is greater than the first
{
	std::string keyline;

	if(!get_line_with_keyword(kwh.word, keyline, req))
		return keywd_missing;

	std::istringstream io_keyline;
	std::string word;
	double doub1,doub2;

	io_keyline.str(keyline);

	io_keyline >> word >> doub1 >> doub2;

	if ((doub1 >= min) && (doub1 <= max) && (doub2 >= min) && (doub2 <= max) && (doub2 > doub1))
	{
		arg1 = doub1;
		arg2 = doub2;
		return 1;
	}

	std::cout << std::endl << "Input Error: " << kwh.word << " contains an invalid parameter." << std::endl;
	std::cout << kwh.hint << std::endl << std::endl;

	return param_invalid;
}
/******************************************************************************/
int InputRead::parse_line_dvect(key_word_help kwh, std::vector<double> &vect_lim, std::vector<double> &vect_val, bool req)
{
	std::string keyline;

	if(!get_line_with_keyword(kwh.word, keyline, req))
		return keywd_missing;

	std::istringstream io_keyline;
	std::string word;
	std::vector<double> vect(vect_val.size());

	io_keyline.str(keyline);

	io_keyline >> word;

	int ok = 0;
	for (int i=0; i<vect.size(); i++) {
		io_keyline >> vect[i];
		if (fabs(vect[i])>fabs(vect_lim[i])) break;
		vect_val[i] = vect[i];
		ok += 1;
	}

	if (ok == vect.size()) return 1;

	std::cout << std::endl << "Input Error: " << kwh.word << " contains an invalid parameter." << std::endl;
	std::cout << kwh.hint << std::endl << std::endl;

	return param_invalid;
}
/******************************************************************************/
int InputRead::parse_line_dvect(key_word_help kwh, double min, double max, std::vector<double> &vect_val, bool req)
{
	std::string keyline;

	if(!get_line_with_keyword(kwh.word, keyline, req))
		return keywd_missing;

	std::istringstream io_keyline;
	std::string word;
	std::vector<double> vect(vect_val.size());

	io_keyline.str(keyline);

	io_keyline >> word;

	int ok = 0;
	for (int i=0; i<vect.size(); i++) {
		io_keyline >> vect[i];
		if ((vect[i]<min)||(vect[i]>max)) break;
		vect_val[i] = vect[i];
		ok += 1;
	}

	if (ok == vect.size()) return 1;

	std::cout << std::endl << "Input Error: " << kwh.word << " contains an invalid parameter." << std::endl;
	std::cout << kwh.hint << std::endl << std::endl;

	return param_invalid;
}
/******************************************************************************/
