
#include "InputRead.h"

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
InputRead::InputRead()
{
	Point min_pt(0,0,0);
	Point max_pt(0,0,0);
	search_box = new BoundBox(min_pt, max_pt);

	// parameter limits
	kwh_ref_fname.good.assign("name of raw image file (in working directory) or a full path");
	kwh_cor_fname.good.assign("name of raw image file (in working directory) or a full path");
	kwh_pts_fname.good.assign("name of tab-txt nxyz file (in working directory) or a full path");
	kwh_out_fname.good.assign("valid file name (for working directory) or a writeable path");

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

	kwh_subvol_size.good.assign("1 <= int <= smallest dimension of the image volumes");

	subvol_npts_min = 1;
	subvol_npts_max = 50000;
	kwh_subvol_npts.good = limits_to_string(subvol_npts_min, subvol_npts_max);

	// keyword subvol_thresh
	ok_subvol_thresh = line_to_vect(ok_on_off_line);
	kwh_subvol_thresh.good = limits_to_string(ok_subvol_thresh);

	min_vol_fract_min = 0.0;
	min_vol_fract_max = 1.0;
	kwh_min_vol_fract.good = limits_to_string(min_vol_fract_min, min_vol_fract_max);

	kwh_gray_thresh_min.good.assign("0 <= int <= 2^vol_bit_depth, and < gray_thresh_max");
	kwh_gray_thresh_max.good.assign("0 <= int <= 2^vol_bit_depth, and > gray_thresh_min");

	kwh_disp_max.good.assign("1 <= int <= smallest dimension of the image volumes");

	ok_num_srch_dof.push_back(3);
	ok_num_srch_dof.push_back(6);
	ok_num_srch_dof.push_back(12);
	kwh_num_srch_dof.good = limits_to_string(ok_num_srch_dof);

	// keyword obj_function
	ok_obj_function = line_to_vect(ok_obj_fcn_line);
	kwh_obj_function.good = limits_to_string(ok_obj_function);

	// keyword interp_type
	ok_interp_type = line_to_vect(ok_interp_mthd_line);
	kwh_interp_type.good = limits_to_string(ok_interp_type);

	kwh_rigid_trans.good.assign("abs(x,y,z) limited to the dimensions of the voxel space");
	kwh_basin_radius.good.assign("0.0 <= double <= disp_max");
	kwh_subvol_aspect.good.assign("0.1 <= double <= 10.0 for each component in (x,y,z)");
	aspect_min = 0.1;
	aspect_max = 10.0;

/*
	// convert to line_to_vect form when implemented
	ok_fine_srch.push_back("powell");
	ok_fine_srch.push_back("levenberg");
	ok_fine_srch.push_back("bfgs");
	ok_fine_srch.push_back("steepest");
	kwh_fine_srch.good = limits_to_string(ok_fine_srch);
*/
	// file name keywords

	kwh_ref_fname.word.assign("reference_filename");
	kwh_ref_fname.exam.assign("reference_volume.raw");
	kwh_ref_fname.reqd.assign("yes");
	kwh_ref_fname.pool.assign("fio_name");
	kwh_ref_fname.hint.assign("### single raster file (raw) of same dimensions as the correlate_volume");
	kwh_ref_fname.help.assign("   Specify the file containing the reference image volume.\n");
	kwh_ref_fname.help.append("   Format as a single uncompressed raw data file with fixed-length (or no) header.\n");
	kwh_ref_fname.help.append("   e.g. ImageJ Stack File -> Save As -> Raw Data ....\n");
	kwh_ref_fname.help.append("   Place the file in the current working directory or include path information.\n");
	kwh_ref_fname.help.append("   Reference and Correlate files most have same dimensionality (header, wide, high, tall parameters).\n");
	kwh_ref_fname.help.append("\n");
	manual.push_back(kwh_ref_fname);

	kwh_cor_fname.word.assign("correlate_filename");
	kwh_cor_fname.exam.assign("correlate_volume.raw");
	kwh_cor_fname.reqd.assign("yes");
	kwh_cor_fname.pool.assign("fio_name");
	kwh_cor_fname.hint.assign("### single raster file (raw) of same dimensions as the reference_volume");
	kwh_cor_fname.help.assign("   Specify the file containing the correlation image volume.\n");
	kwh_cor_fname.help.append("   Format as a single uncompressed raw data file with fixed-length (or no) header.\n");
	kwh_cor_fname.help.append("   e.g. ImageJ Stack File -> Save As -> Raw Data ....\n");
	kwh_cor_fname.help.append("   Place the file in the current working directory or include path information.\n");
	kwh_cor_fname.help.append("   Reference and Correlate files most have same dimensionality (header, wide, high, tall parameters).\n");
	kwh_cor_fname.help.append("\n");
	manual.push_back(kwh_cor_fname);

	kwh_pts_fname.word.assign("point_cloud_filename");
	kwh_pts_fname.exam.assign("ROI_points_nxyz.txt");
	kwh_pts_fname.reqd.assign("yes");
	kwh_pts_fname.pool.assign("fio_name");
	kwh_pts_fname.hint.assign("### tab delimited text file containing Region of Interest point labels (n) and (xyz) loctions");
	kwh_pts_fname.help.assign("   Specify the file containing data for all measurement points in the Region of Interest (ROI).\n");
	kwh_pts_fname.help.append("   The ROI is defined as a cloud of points that fill a geometric region within the reference volume.\n");
	kwh_pts_fname.help.append("   Point cloud size, shape, and density are completely flexible, as long as all points fall within the image volumes.\n");
	kwh_pts_fname.help.append("   Dense point clouds that accurately reflect sample geometry and reflect measurement objectives yield the best results.\n");
	kwh_pts_fname.help.append("   Finite element meshing or other geometry discretization software is very useful for creating point clouds.\n");
	kwh_pts_fname.help.append("   Each line in the tab delimited file contains an integer point label followed by the x,y,z point location, e.g.\n");
	kwh_pts_fname.help.append("      1   300   750.2  208 \n");
	kwh_pts_fname.help.append("      2   300   750.2  209 \n");
	kwh_pts_fname.help.append("      etc. \n");
	kwh_pts_fname.help.append("   Non-integer voxel locations are admitted, with reference volume interpolation used as needed.\n");
	kwh_pts_fname.help.append("   The first point is significant, as it is used as a global starting point and reference for the rigid_trans variable.\n");
	kwh_pts_fname.help.append("   Place the file in the current working directory or include path information.\n");
	kwh_pts_fname.help.append("\n");
	manual.push_back(kwh_pts_fname);

	kwh_out_fname.word.assign("output_filename");
	kwh_out_fname.exam.assign("\tmyproject_results");
	kwh_out_fname.reqd.assign("yes");
	kwh_out_fname.pool.assign("fio_name");
	kwh_out_fname.hint.assign("### base name for run status (.stat) and displacement data (.disp) output files");
	kwh_out_fname.help.assign("   Specify a base output file name for results of dvc code execution.\n");
	kwh_out_fname.help.append("   If a filename alone is given, output files are placed in the current working directory.\n");
	kwh_out_fname.help.append("   Alternatively, a full Unix-style path can precede the filename.\n");
	kwh_out_fname.help.append("   The terminal window owner must have write permission for the target directory.\n");
	kwh_out_fname.help.append("\n");
	manual.push_back(kwh_out_fname);

	// image volume raw data parameters

	kwh_vol_bit_depth.word.assign("vol_bit_depth");
	kwh_vol_bit_depth.exam.assign("8");
	kwh_vol_bit_depth.reqd.assign("yes");
	kwh_vol_bit_depth.pool.assign("vox_data");
	kwh_vol_bit_depth.hint.assign("### 8 or 16");
	kwh_vol_bit_depth.help.assign("   Defines the bit depth of both reference and correlate image volumes.\n");
	kwh_vol_bit_depth.help.append("   Assumes unsigned integer voxel representation.\n");
	kwh_vol_bit_depth.help.append("\n");
	manual.push_back(kwh_vol_bit_depth);

	kwh_vol_endian.word.assign("vol_endian");
	kwh_vol_endian.exam.assign("little");
	kwh_vol_endian.reqd.assign("If vol_bit_depth is not 8");
	kwh_vol_endian.pool.assign("vox_data");
	kwh_vol_endian.hint.assign("### little or big, ignored if vol_bit_depth is 8");
	kwh_vol_endian.help.assign("   Defines the byte organization for 16-bit image volumes.\n");
	kwh_vol_endian.help.append("   Find the appropriate setting using ImageJ after cross-platform image transfers.\n");
	kwh_vol_endian.help.append("   Big generates a byte swap: Little preserves the byte order.\n");
	kwh_vol_endian.help.append("\n");
	manual.push_back(kwh_vol_endian);

	kwh_vol_hdr_lngth.word.assign("vol_hdr_lngth");
	kwh_vol_hdr_lngth.exam.assign("0");
	kwh_vol_hdr_lngth.reqd.assign("yes");
	kwh_vol_hdr_lngth.pool.assign("vox_data");
	kwh_vol_hdr_lngth.hint.assign("### fixed-length header size, may be zero");
	kwh_vol_hdr_lngth.help.assign("   Defines a fixed header length for both reference and correlate image volumes.\n");
	kwh_vol_hdr_lngth.help.append("   This value will often be 0, as is the case for standard raw data files.\n");
	kwh_vol_hdr_lngth.help.append("\n");
	manual.push_back(kwh_vol_hdr_lngth);

	kwh_vol_wide.word.assign("vol_wide");
	kwh_vol_wide.exam.assign("4008");
	kwh_vol_wide.reqd.assign("yes");
	kwh_vol_wide.pool.assign("vox_data");
	kwh_vol_wide.hint.assign("### width in pixels of each slice");
	kwh_vol_wide.help.assign("   Defines the width of slices in both reference and correlate image volumes.\n");
	kwh_vol_wide.help.append("   Wide = x for the coordinate system used by the dvc code, origin at the left.\n");
	kwh_vol_wide.help.append("\n");
	manual.push_back(kwh_vol_wide);

	kwh_vol_high.word.assign("vol_high");
	kwh_vol_high.exam.assign("2670");
	kwh_vol_high.reqd.assign("yes");
	kwh_vol_high.pool.assign("vox_data");
	kwh_vol_high.hint.assign("### height in pixels of each slice");
	kwh_vol_high.help.assign("   Defines the height of slices in both reference and correlate image volumes.\n");
	kwh_vol_high.help.append("   High = y for the coordinate system used by the dvc code, origin at the top.\n");
	kwh_vol_high.help.append("\n");
	manual.push_back(kwh_vol_high);

	kwh_vol_tall.word.assign("vol_tall");
	kwh_vol_tall.exam.assign("4016");
	kwh_vol_tall.reqd.assign("yes");
	kwh_vol_tall.pool.assign("vox_data");
	kwh_vol_tall.hint.assign("### number of slices in the stack");
	kwh_vol_tall.help.assign("   Defines the number of slices in both reference and correlate image volumes.\n");
	kwh_vol_tall.help.append("   Tall = z for the coordinate system used by the dvc code, origin is the first slice.\n");
	kwh_vol_tall.help.append("\n");
	manual.push_back(kwh_vol_tall);

	// subvolume parameters

	kwh_subvol_geom.word.assign("subvol_geom");
	kwh_subvol_geom.exam.assign("cube");
	kwh_subvol_geom.reqd.assign("yes");
	kwh_subvol_geom.pool.assign("sub_vols");
	kwh_subvol_geom.hint.assign("### cube, sphere: geometry of the subvolumes");
	kwh_subvol_geom.help.assign("   Defines geometry of the subvolumes created around each search point.\n");
	kwh_subvol_geom.help.append("   Cubes are the classic, but essentially arbitrary, subvolume geometry.\n");
	kwh_subvol_geom.help.append("   Try spheres if portions of a point cloud are near a surface or interface.\n");
	kwh_subvol_geom.help.append("\n");
	manual.push_back(kwh_subvol_geom);

	kwh_subvol_size.word.assign("subvol_size");
	kwh_subvol_size.exam.assign("30");
	kwh_subvol_size.reqd.assign("yes");
	kwh_subvol_size.pool.assign("sub_vols");
	kwh_subvol_size.hint.assign("### side length or diameter, in voxels");
	kwh_subvol_size.help.assign("   Defines size (in voxels) of the subvolumes created around each search point.\n");
	kwh_subvol_size.help.append("   Values indicate a characteristic dimension (edge length or diameter) of the subvolumes.\n");
	kwh_subvol_size.help.append("   This parameter is completely independent of subvol_npts.\n");
	kwh_subvol_size.help.append("\n");
	manual.push_back(kwh_subvol_size);

	kwh_subvol_npts.word.assign("subvol_npts");
	kwh_subvol_npts.exam.assign("5000");
	kwh_subvol_npts.reqd.assign("yes");
	kwh_subvol_npts.pool.assign("sub_vols");
	kwh_subvol_npts.hint.assign("### number of points to distribute within the subvol");
	kwh_subvol_npts.help.assign("   Defines the number of points within each subvolume (max is 50000).\n");
	kwh_subvol_npts.help.append("   In this code, subvolume point locations are NOT voxel-centered and the number is INDEPENDENT of subvolume size.\n");
	kwh_subvol_npts.help.append("   Interpolation within the reference image volume is used to establish templates with arbitrary point locations.\n");
	kwh_subvol_npts.help.append("   For cubes a uniform grid of approximately subvol_npts is generated.\n");
	kwh_subvol_npts.help.append("   For spheres subvol_npts are randomly distributed within the subvolume.\n");
	kwh_subvol_npts.help.append("   This parameter has a strong effect on computation time, so be careful.\n");
	kwh_subvol_npts.help.append("\n");
	manual.push_back(kwh_subvol_npts);

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

	// required optimization parameters (basic search process)

	kwh_disp_max.word.assign("disp_max");
	kwh_disp_max.exam.assign("15");
	kwh_disp_max.reqd.assign("yes");
	kwh_disp_max.pool.assign("opt_mthd");
	kwh_disp_max.hint.assign("### maximum displacement in voxels, for range checking and search limits");
	kwh_disp_max.help.assign("   Defines the maximum displacement expected within the reference image volume.\n");
	kwh_disp_max.help.append("   This is a very important paramater used for search process control and memory allocation.\n");
	kwh_disp_max.help.append("   Set to a reasonable value just greater than the actual sample maximum displacement.\n");
	kwh_disp_max.help.append("   Be cautious: large displacements make the search process slower and less reliable.\n");
	kwh_disp_max.help.append("   It is best to reduce large rigid body displacements through image volume manipulation.\n");
	kwh_disp_max.help.append("   Future code development will introduce methods for better management of large displacements.\n");
	kwh_disp_max.help.append("\n");
	manual.push_back(kwh_disp_max);

	kwh_num_srch_dof.word.assign("num_srch_dof");
	kwh_num_srch_dof.exam.assign("6");
	kwh_num_srch_dof.reqd.assign("yes");
	kwh_num_srch_dof.pool.assign("opt_mthd");
	kwh_num_srch_dof.hint.assign("### 3 (translation), 6 (+rotation), or 12 (+strain)");
	kwh_num_srch_dof.help.assign("   Defines the degree-of-freedom set for the final stage of the search.\n");
	kwh_num_srch_dof.help.append("   The actual search process introduces degrees-of-freedom in stages up to this value.\n");
	kwh_num_srch_dof.help.append("   Translation only suffices for a quick, preliminary investigation.\n");
	kwh_num_srch_dof.help.append("   Adding rotation will significantly improve displacement accuracy in most cases.\n");
	kwh_num_srch_dof.help.append("   Reserve strain degrees-of-freedom for cases when the highest precision is required.\n");
	kwh_num_srch_dof.help.append("   3  = translation only\n");
	kwh_num_srch_dof.help.append("   6  = translation plus rotation\n");
	kwh_num_srch_dof.help.append("   12 = translation, rotation and strain\n");
	kwh_num_srch_dof.help.append("\n");
	manual.push_back(kwh_num_srch_dof);

	kwh_obj_function.word.assign("obj_function");
	kwh_obj_function.exam.assign("znssd");
	kwh_obj_function.reqd.assign("yes");
	kwh_obj_function.pool.assign("opt_mthd");
	kwh_obj_function.hint.assign("### sad, ssd, zssd, nssd, znssd");
	kwh_obj_function.help.assign("   Defines the objective function template matching form.\n");
	kwh_obj_function.help.append("   See B. Pan, Equivalence of Digital Image Correlation Criteria for Pattern Matching, 2010 \n");
	kwh_obj_function.help.append("   Functions become increasingly expensive and more robust as you progress from sad to znssd.\n");
	kwh_obj_function.help.append("   Minimizing squared-difference and maximizing cross-correlation are functionally equivalent.\n");
	kwh_obj_function.help.append("   sad  = sum of absolute differences\n");
	kwh_obj_function.help.append("   ssd  = sum of squared differences\n");
	kwh_obj_function.help.append("   zssd  = intensity offset insensitive sum of squared differences (value not normalized)\n");
	kwh_obj_function.help.append("   nssd  = intensity range insensitive sum of squared differences (0.0 = perfect match, 1.0 = max value)\n");
	kwh_obj_function.help.append("   znssd  = intensity offset and range insensitive sum of squared differences (0.0 = perfect match, 1.0 = max value)\n");
	kwh_obj_function.help.append("   Notes on objective function values:\n");
	kwh_obj_function.help.append("      1. The normalized quantities nssd and znssd are preferred, as quality of match can be assessed.\n");
	kwh_obj_function.help.append("      2. The natural range of nssd is [0.0 to 2.0], and of znssd is [0.0 to 4.0].\n");
	kwh_obj_function.help.append("      3. Both are scaled for output into the [0.0 to 1.0] range for ease of comparison.\n");

	kwh_obj_function.help.append("\n");
	manual.push_back(kwh_obj_function);

	kwh_interp_type.word.assign("interp_type");
	kwh_interp_type.exam.assign("tricubic");
	kwh_interp_type.reqd.assign("yes");
	kwh_interp_type.pool.assign("opt_mthd");
	kwh_interp_type.hint.assign("### trilinear (faster), tricubic (recommended)");
	kwh_interp_type.help.assign("   Defines the interpolation method used during template matching.\n");
	kwh_interp_type.help.append("   Trilinear is significantly faster, but with known template matching artifacts.\n");
	kwh_interp_type.help.append("   Trilinear is most useful for tuning other search parameters during preliminary runs.\n");
	kwh_interp_type.help.append("   Tricubic is computationally expensive, but is the choice if strain is of interst.\n");
	kwh_interp_type.help.append("\n");
	manual.push_back(kwh_interp_type);

	// optimization tuning parameters

	kwh_rigid_trans.word.assign("rigid_trans");
	kwh_rigid_trans.exam.assign("0.0 0.0 0.0");
	kwh_rigid_trans.reqd.assign("no");
	kwh_rigid_trans.pool.assign("opt_tune");
	kwh_rigid_trans.hint.assign("### x,y,z voxel offset of target volume from reference volume at first point in ROI cloud");
	kwh_rigid_trans.help.assign("   A rigid body offset between reference and target volumes complicates template matching.\n");
	kwh_rigid_trans.help.append("   If you have done a rigid body registration do not use this option, or set to (0.0,0.0,0.0).\n");
	kwh_rigid_trans.help.append("   If you have not, but know the offset, then provide the displacement vector here.\n");
	kwh_rigid_trans.help.append("   Use the FIRST POINT of the ROI cloud for rigid body registration or rigid_trans specification.\n");
	kwh_rigid_trans.help.append("   Notes: \n");
	kwh_rigid_trans.help.append("     1. Only do rigid body registration with whole voxel translations, do not use interpolation.\n");
	kwh_rigid_trans.help.append("     2. The first point of the ROI cloud is used as a global starting point and therefore as a translation reference.\n");
	kwh_rigid_trans.help.append("\n");
	manual.push_back(kwh_rigid_trans);

	kwh_basin_radius.word.assign("basin_radius");
	kwh_basin_radius.exam.assign("0.0");
	kwh_basin_radius.reqd.assign("no");
	kwh_basin_radius.pool.assign("opt_tune");
	kwh_basin_radius.hint.assign("### coarse search resolution (voxels): default = 0.0, max = disp_max");

	kwh_basin_radius.help.assign("   The default process uses cloud points sorted from a global start to establish initial disp values.\n");
	kwh_basin_radius.help.append("   To conduct a coarse search in addition (gridded translation), set basin_radius to a non-zero value.\n");
	kwh_basin_radius.help.append("   The basin_radius value sets the coarse-search resolution (grid step size).\n");
	kwh_basin_radius.help.append("   If basin_radius is not specified, the default value of 0.0 voxels is set and no coarse search is done.\n");
	kwh_basin_radius.help.append("   A small basin_radius (< 1 voxel) will cause very slow execution if disp_max is large.\n");
	kwh_basin_radius.help.append("   A large basin_radius (> 5 voxels) will cause unreliable matching if subvol_size is small.\n");
	kwh_basin_radius.help.append("   Use the ImageJ plugin 'cv_Match_Template' to explore this parameter.\n");
	kwh_basin_radius.help.append("\n");
	manual.push_back(kwh_basin_radius);

	kwh_subvol_aspect.word.assign("subvol_aspect");
	kwh_subvol_aspect.exam.assign("1.0 1.0 1.0");
	kwh_subvol_aspect.reqd.assign("no");
	kwh_subvol_aspect.pool.assign("opt_tune");
	kwh_subvol_aspect.hint.assign("### stretch or contract the subvolume in any coordinate direction between 0.1 and 10.0");
	kwh_subvol_aspect.help.assign("   A standard subvolume is isotropic, with equivalent size in each coordinate direction.\n");
	kwh_subvol_aspect.help.append("   This parameter describes a change in shape of the subvolume to accomodate elongated texture.\n");
	kwh_subvol_aspect.help.append("   It is only useful if the texture direciton is consistent, and alligned with a coordinate direction.\n");
	kwh_subvol_aspect.help.append("   The aspect change is specified as three doubles, indicating stretch/contract in the coordinate directions.\n");
	kwh_subvol_aspect.help.append("   A default of 1.0 1.0 1.0 is used if the parameter is not included in the input file.\n");
	kwh_subvol_aspect.help.append("   (Function slated for automated and localized setting in future versions)\n");
	kwh_subvol_aspect.help.append("\n");
	manual.push_back(kwh_subvol_aspect);

/*
	kwh_fine_srch.word.assign("fine_search");
	kwh_fine_srch.exam.assign("bfgs");
	kwh_fine_srch.reqd.assign("yes");
	kwh_fine_srch.pool.assign("opt_mthd");
	kwh_fine_srch.hint.assign("### powell, levenberg, bfgs, steepest");
	kwh_fine_srch.help.assign("\tDefines the method for high-precision (fine) optimization.\n");
	kwh_fine_srch.help.append("\tIt is the final optimization step, after starting point and base search processes.\n");
	kwh_fine_srch.help.append("\tvalues: powell, levenberg, bfgs, steepest\n");
	kwh_fine_srch.help.append("\n");
	manual.push_back(kwh_fine_srch);
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

	if(parse_line_old_file(kwh_ref_fname, run->ref_fname, ref_file_length, true) != input_line_ok ) return 0;
	if(parse_line_old_file(kwh_cor_fname, run->cor_fname, cor_file_length, true) != input_line_ok ) return 0;
	if(parse_line_old_file(kwh_pts_fname, run->pts_fname, pts_file_length, true) != input_line_ok ) return 0;
	if(parse_line_new_file(kwh_out_fname, run->out_fname, true) != 1 ) return 0;

	run->res_fname = run->out_fname + ".disp";
	run->sta_fname = run->out_fname + ".stat";

	if(parse_line_vec_val(kwh_vol_bit_depth, ok_vol_bit_depth, run->vol_bit_depth, true) != input_line_ok ) return 0;
	if(run->vol_bit_depth != 8)
	{
		if(parse_line_vec_val(kwh_vol_endian, ok_vol_endian, run->vol_endian, true) != input_line_ok ) return 0;
	}

	if(parse_line_min_max(kwh_vol_hdr_lngth, 0, vol_hdr_max, run->vol_hdr_lngth, true) != input_line_ok ) return 0;
	if(parse_line_min_max(kwh_vol_wide, 0, vol_dim_max, run->vol_wide, true) != input_line_ok ) return 0;
	if(parse_line_min_max(kwh_vol_high, 0, vol_dim_max, run->vol_high, true) != input_line_ok ) return 0;
	if(parse_line_min_max(kwh_vol_tall, 0, vol_dim_max, run->vol_tall, true) != input_line_ok ) return 0;

	subvol_size_max = run->vol_wide;	// used for limiting subvol_size and disp_max
	if (run->vol_high < subvol_size_max) subvol_size_max = run->vol_high;
	if (run->vol_tall < subvol_size_max) subvol_size_max = run->vol_tall;

	if(parse_line_vec_val(kwh_subvol_geom, ok_subvol_geom, run->subvol_geom, true) != input_line_ok ) return 0;
	for (int i=0; i<ok_subvol_geom.size(); i++)
		if (run->subvol_geom == ok_subvol_geom[i]) run->sub_geo = (Subvol_Type)i;
	if(parse_line_min_max(kwh_subvol_size, 0, subvol_size_max, run->subvol_size, true) != input_line_ok ) return 0;
	if(parse_line_min_max(kwh_subvol_npts, 0, subvol_npts_max, run->subvol_npts, true) != input_line_ok ) return 0;
	if(parse_line_vec_val(kwh_subvol_thresh, ok_subvol_thresh, run->subvol_thresh, true) != input_line_ok ) return 0;

	if(run->subvol_thresh == "on")
	{
		if(parse_line_min_max(kwh_gray_thresh_min, 0, pow((double)2,(double)run->vol_bit_depth), run->gray_thresh_min, true) != input_line_ok ) return 0;
		if(parse_line_min_max(kwh_gray_thresh_max, run->gray_thresh_min, pow((double)2,(double)run->vol_bit_depth), run->gray_thresh_max, true) != input_line_ok ) return 0;
		if(parse_line_min_max(kwh_min_vol_fract, 0.0, min_vol_fract_max, run->min_vol_fract, true) != input_line_ok ) return 0;
	}

	if(parse_line_min_max(kwh_disp_max, 0, subvol_size_max, run->disp_max, true) != input_line_ok ) return 0;
	if(parse_line_vec_val(kwh_num_srch_dof, ok_num_srch_dof, run->num_srch_dof, true) != input_line_ok ) return 0;
	if(parse_line_vec_val(kwh_obj_function, ok_obj_function, run->obj_function, true) != input_line_ok ) return 0;
	for (int i=0; i<ok_obj_function.size(); i++)
		if (run->obj_function == ok_obj_function[i]) run->obj_fcn = (Objfcn_Type)i;
	if(parse_line_vec_val(kwh_interp_type, ok_interp_type, run->interp_type, true) != input_line_ok ) return 0;
	for (int i=0; i<ok_interp_type.size(); i++)
		if (run->interp_type == ok_interp_type[i]) run->int_typ = (Interp_Type)i;

	// optional parameters

	run->basin_radius = 0.0;
	if(parse_line_min_max(kwh_basin_radius, 0, run->disp_max, run->basin_radius, false) == param_invalid) return 0;

	run->rigid_trans.resize(3, 0.0);
	std::vector<double> rigid_trans_limit(3);
	rigid_trans_limit[0] = (double)run->vol_wide;
	rigid_trans_limit[1] = (double)run->vol_high;
	rigid_trans_limit[2] = (double)run->vol_tall;
	if(parse_line_dvect(kwh_rigid_trans, rigid_trans_limit, run->rigid_trans, false) == param_invalid) return 0;

	run->subvol_aspect.resize(3, 1.0);
	if(parse_line_dvect(kwh_subvol_aspect, aspect_min, aspect_max, run->subvol_aspect, false) == param_invalid) return 0;

	// check image volumes
	unsigned long expected_vol_file_size = (unsigned long) run->vol_hdr_lngth + (unsigned long) run->vol_wide *  (unsigned long) run->vol_high * (unsigned long) run->vol_tall * (unsigned long) (run->vol_bit_depth / 8);

	if (ref_file_length != expected_vol_file_size)
	{
		std::cout << "\n";
		std::cout << "Reference volume file size does not match the volume description.\n";
		std::cout << "Check vol_bit_depth, vol_hdr_lngth, vol_high, vol_wide, and vol_tall.\n";
		std::cout << "\n";
		return 0;
	}

	if (cor_file_length != expected_vol_file_size)
	{
		std::cout << "\n";
		std::cout << "Correlate volume file size does not match the volume description.\n";
		std::cout << "Check vol_bit_depth, vol_hdr_lngth, vol_high, vol_wide, and vol_tall.\n";
		std::cout << "\n";
		return 0;
	}

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
		std::cout << "\n";
		std::cout << "No points were read, the Point Cloud file may be improperly formatted.\n";
		std::cout << "The expected format is plain text, tab (or other 'white space') delimited.\n";
		std::cout << "Header info is OK as long as it does not match the format of a point description.\n";
		std::cout << "Each line of the file should contain an integer point label and x,y,z coordinates, e.g.:\n\n";
		std::cout << "5\t10.72\t15.87\t23.45\n\n";
		std::cout << "\n";
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
	//		if(ss.peek() == '\r') {
	//			std::cout << "found slash r" << std::endl;
	//		}
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

	while (getline(ifs, line)) {
		std::stringstream ss(line);

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

	}

	return 1;
}
/******************************************************************************/
int InputRead::print_manual_intro(std::ofstream &file)
{
	file << "#####################################################################\n";
	file << "###                                                               ###\n";
	file << "###   A brief (and hopefully useful) manual for the dvc code      ###\n";
	file << "###                                                               ###\n";
	file << "#####################################################################\n";
	file << "\n";
	file << "(Best viewed in a simple text editor with a fixed-width font and no line wrapping.)\n";
	file << "\n";

	file << "Copyright 2014 Brian K. Bay (computer code and all documentation)\n";
	file << "\n";

	file << "Created:  1 Jan 2014\n";
	file << "Revised:  " << DAY_REV << " " << MONTH_REV << " " << YEAR_REV << "\n";
	file << "version:  " << VERSION << "\n";
	file << "\n";

	file << "The dvc code is written in c++ with portability and simple compilation in mind.\n";
	file << "At present only an single external library (eigen) is used for sparse matrix interpolation calculations.\n";
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
	file << "Some keywords are only reguired if other keywords have particular values.\n";
	file << "These CONDITIONAL KEYWORDS are ignored if they are left within an input file but are not needed.\n";
	file << "\n";
	file << "The keywords are organized into four groups:\n";
	file << "\n";
	file << "   fio_name \t\t   // names of existing files and files created during a run\n";
	file << "   vox_data \t\t   // description of the voxel data files that are targeted for analysis\n";
	file << "   sub_vols \t\t   // description of the subvolumes created for the template matching process\n";
	file << "   opt_mthd \t\t   // REQUIRED parameters for the default optimization (template matching) process\n";
	file << "   opt_tune \t\t   // OPTIONAL parameters for tuning and refining the template matching process\n";
	file << "\n";

	file << "The keywords are described below, with informaiton organized as:\n";
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
			file << "\texamplar:\t" << manual[i].word << "\t" << manual[i].exam << "\n";
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
	file << "   - The point identifier appears first, folowed by the [x,y,z] location.\n";
	file << "   - The search status appears next.\n";
	file << "      - Point_Good = successful search convergence within the max displacement.\n";
	file << "      - Range_Fail = max displacement exceeded; consider increasing the disp_max parameter.\n";
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
	file << "      - A tab-deliminited text file of the dvc results.\n";
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

	sta_file << kwh_subvol_thresh.word << "\t" << run->subvol_thresh  << "\n";
	if(run->subvol_thresh == "on")
	{
		sta_file << kwh_gray_thresh_min.word << "\t" << run->gray_thresh_min  << "\n";
		sta_file << kwh_gray_thresh_max.word << "\t" << run->gray_thresh_max  << "\n";
		sta_file << kwh_min_vol_fract.word << "\t" << run->min_vol_fract << "\n";
	}
	sta_file << "\n";

	// opt_mthd

	sta_file << kwh_disp_max.word << "\t"<< run->disp_max << "\n";
	sta_file << kwh_num_srch_dof.word << "\t"<< run->num_srch_dof << "\n";
	sta_file << kwh_obj_function.word << "\t"<< run->obj_function << "\n";
	sta_file << kwh_interp_type.word << "\t"<< run->interp_type << "\n";
	sta_file << "\n";

	// opt_tune

	sta_file << kwh_rigid_trans.word << "\t" << run->rigid_trans[0] << "\t" << run->rigid_trans[1] << "\t" << run->rigid_trans[2] << "\n";
	sta_file << kwh_basin_radius.word << "\t" << run->basin_radius << "\n";
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

	res_file << n;

	res_file << "\t" << pt.x() << "\t" << pt.y() << "\t" << pt.z();

	res_file << "\t" << status;

	res_file << "\t" << obj_min;

	res_file << std::fixed << std::setprecision(6);

	// this prints full search params, disp, rotation, strain if used
	/*
	for (int i=0; i<result.size(); i++)
	{
		res_file << "\t" << result[i];
	} 
	*/

	// this prints just the dispalcements
	for (int i=0; i<3; i++)
	{
		res_file << "\t" << result[i];
	} 

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

	std::cout << "\nInput Error: " << kwh.word << " cannot find file: " << str1 << "\n\n";
	std::cout << kwh.hint << "\n\n" << kwh.help;

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

	std::cout << "\nInput Error: " << kwh.word << " contains an invalid parameter.\n\n";
	std::cout << kwh.hint << "\n\n" << kwh.help;

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

	std::cout << "\nInput Error: " << kwh.word << " contains an invalid parameter.\n\n";
	std::cout << kwh.hint << "\n\n" << kwh.help;

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

	std::cout << "\nInput Error: " << kwh.word << " contains an invalid parameter.\n\n";
	std::cout << kwh.hint << "\n\n" << kwh.help;

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

	std::cout << "\nInput Error: " << kwh.word << " contains an invalid parameter.\n\n";
	std::cout << kwh.hint << "\n\n" << kwh.help;

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

	std::cout << "\nInput Error: " << kwh.word << " contains an invalid parameter.\n\n";
	std::cout << kwh.hint << "\n\n" << kwh.help;

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

	std::cout << "\nInput Error: " << kwh.word << " contains an invalid parameter.\n\n";
	std::cout << kwh.hint << "\n\n" << kwh.help;

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

	std::cout << "\nInput Error: " << kwh.word << " contains an invalid parameter.\n\n";
	std::cout << kwh.hint << "\n\n" << kwh.help;

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

	std::cout << "\nInput Error: " << kwh.word << " contains an invalid parameter.\n\n";
	std::cout << kwh.hint << "\n\n" << kwh.help;

	return param_invalid;
}
/******************************************************************************/
