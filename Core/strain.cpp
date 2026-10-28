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

#include "strain.h"

/*
Testing branch pushing and pulling between local repos (e.g. Mac, Linux)
This comment inserted on core_updates branch, Linux repo, 11 Feb 2022.
*/

/******************************************************************************/
StrainCalc::StrainCalc ()
{
	num_mod_para = 10;

	num_data_points_min = 10;		// min number for the strain window size
	num_data_points_def = 25;		// default number for the strain window size
	// max comes from DataCloud .sort file setting

	objmin_thresh_min = 0.0001;		// only highly matched points used
	objmin_thresh_max = 1.0;		// all points used
	objmin_thresh_def = 1.0;		// default is no thresholding
}

/******************************************************************************/

/******************************************************************************/

int main(int argc, char *argv[])
{
	std::cout << std::endl;

	StrainCalc strain;		// instantiate StrainCalc object to set fixed run parameters and default values
	DataCloud data;			// instantiate DataCloud object to access .sort file settings and set up storage
	InputRead in;			// instantiate InputRead to access command line parsing and .disp file reader

	int ndp_min = strain.ndp_min();
	int ndp_max = data.nbr_num_save();		// this coordinates with .sort file management
	int ndp = strain.ndp_def();

	double objt_min = strain.objt_min();
	double objt_max = strain.objt_max();
	double objt = strain.objt_def();

	std::string fname_disp;
	std::string fname_base;
	std::string fname_sort_read;

	// all of these may be modified by command line inputs
	bool do_sort_read = false;
	bool refill = false;
	bool xy_only = false;	// strain calc limitations for coordinate plane point clouds
	bool yz_only = false;
	bool zx_only = false;

	// output file options, GrLa is the default (turned off with -Loff, -L redundantly offered as well)
	bool out_Biot = false;
	bool out_GrLa = true;
	bool out_Henc = false;
	bool out_dgrd = false;
	bool out_rvec = false;

	// command entered with no arguments
	if (argc == 1)
	{
		std::cout << "Strain command needs more input." << std::endl;
		std::cout << std::endl << "Command line:" << std::endl;
		std::cout << "strain dvc.disp.csv" << std::endl;
		std::cout << "  • use dvc output file dvc.disp.csv, create sort file, use default run settings"  << std::endl;
		std::cout << "  • it is much more efficient to read an eisting .sort file, particularly for large point clouds"  << std::endl;
		std::cout << "  • but the strain command will run with only a .disp file, creating a .sort file in the process for subsequent use"  << std::endl;
		std::cout << "strain dvc.disp.csv dvc.sort.csv" << std::endl;
		std::cout << "  • use dvc output file dvc.disp.csv and sort file dvc.sort.csv, use default run settings"  << std::endl;
		std::cout << "  • use either a .sort file created during the dvc run or one created during an initial strain run"  << std::endl;

		std::cout << std::endl << "Default settings:" << std::endl;
		std::cout << "strain window = " << strain.ndp_def() << " points (min = " << strain.ndp_min() << ", max = " << data.nbr_num_save() << ")" << std::endl;
		std::cout << "  • reset with -sw flag" << std::endl;
		std::cout << "objmin threshold = " << strain.objt_def() << " (min = " << strain.objt_min() << ", max = " << strain.objt_def() << ")" << std::endl;
		std::cout << "  • values lower than the threshold are used in filling the strain window" << std::endl;
		std::cout << "  • the default value of 1.0 passes all convergent points into the strain window" << std::endl;
		std::cout << "  • reset with -t flag" << std::endl;
		std::cout << "output Lagrangian strain values" << std::endl;
		std::cout << "  • adjust output with -E, -D, and -A flags" << std::endl;

		std::cout << std::endl << "Command line flags:" << std::endl;
		std::cout << "-sw 30" << "\t\t" <<  " • specify a target number of points for the strain windows (e.g. 30)" << std::endl;
		std::cout << "-t 0.01" << "\t\t" <<  " • specify the objmin threshold value (e.g. 0.01)" << std::endl;
		std::cout << "-r" << "\t\t" <<  " • refill strain window to replace points failing threshold (may expand strain window size)" << std::endl;
		std::cout << "-E" << "\t\t" <<	" • add engineering strain output (default is Lagrangian)" << std::endl;
		std::cout << "-D" << "\t\t" <<	" • add displacement gradient tensor output" << std::endl;
		std::cout << "-A" << "\t\t" <<	" • write all output files" << std::endl;
		std::cout << std::endl;
		return 0;
	}

	// process command line flags first
	// look for the flag, adjust run settings, then take it out and adjust argc and argv accordingly

	// run control settings
	if (in.find_flag("-sw", argc, argv, ndp)) {	// strain window number of points
		if (ndp < ndp_min) ndp = ndp_min;
		if (ndp > ndp_max) ndp = ndp_max;
	}

	if (in.find_flag("-t", argc, argv, objt)) { // objmin threshold
		if (objt < objt_min) objt = objt_min;
		if (objt > objt_max) objt = objt_max;
	}

	if (in.find_flag("-r", argc, argv)) {		// use strain window refill option
		refill = true;
	}

	// this manually sets to a 2d coordinate plane analysis
	// code later uses an automated method based on cloud point position data
	// leaving these flags in place but hidden, not listed in "no arg" output or manual

	if (in.find_flag("-xy", argc, argv)) {		// do 2d strain calc for xy plane point cloud
		xy_only = true;
	}

	if (in.find_flag("-yz", argc, argv)) {		// do 2d strain calc for yz plane point cloud
		yz_only = true;
	}

	if (in.find_flag("-zx", argc, argv)) {		// do 2d strain calc for yz plane point cloud
		zx_only = true;
	}
	// end of planar flags

	// output file control
	
	// the default output is Lagrangian strains, allow users to turn off
	if (in.find_flag("-Loff", argc, argv)) {
		out_GrLa = false;
	}

	// doesn't hurt to add this as well, users may expect it
	if (in.find_flag("-L", argc, argv)) {
		out_GrLa = true;
	}	

	if (in.find_flag("-B", argc, argv)) {
		out_Biot = true;
	}

	if (in.find_flag("-H", argc, argv)) {
		out_Henc = true;
	}

	if (in.find_flag("-D", argc, argv)) {
		out_dgrd = true;
	}

	if (in.find_flag("-R", argc, argv)) {
		out_rvec = true;
	}

	if (in.find_flag("-A", argc, argv)) {
		out_Biot = true;
		out_GrLa = true;
		out_Henc = true;
		out_dgrd = true;
		out_rvec = true;
	}

	// clear any extraneous flags, find_flag writes an output message if found
	in.find_flag(0, 1, "-", argc, argv);

	// argc and argv now contain just possible input file names
	// look for a readable file as the first argument, set as .disp file
	if (argc > 1)
	{
		fname_disp = argv[1];
		std::ifstream input_disp;
		input_disp.open(fname_disp.c_str());
		if (!input_disp.good()){
			std::cout << "-> cannot find .disp file '" << fname_disp << std::endl;
			return 0;
		}
		input_disp.close();
	}

	// use .disp file name to develop output file names
	// assumes .disp file name has no extension or one or more .extensions
	// work with the name up to the first extension to form output file names

	std::cout << "using disp file: " << fname_disp << std::endl;
	size_t fname_disp_length = fname_disp.length();
	size_t fname_disp_to_dot = fname_disp.find_first_of(".");
	if (fname_disp_to_dot == string::npos) {	// no dots in file name
		fname_base = fname_disp;
	} else {									// remove everything past first dot for base name
		fname_base = fname_disp;
		fname_base.erase(fname_base.end() - (fname_disp_length - fname_disp_to_dot), fname_base.end());
	}

	// look for a readable file as the second argument, set as .sort file
	if (argc > 2)
	{
		fname_sort_read = argv[2];
		std::ifstream input_sort_read;
		input_sort_read.open(fname_sort_read.c_str());
		if (input_sort_read.good())
		{
			do_sort_read = true;
		} else {
			std::cout << "-> cannot find .sort file " << fname_sort_read << std::endl;
			do_sort_read = false;
		}
		input_sort_read.close();
	}

	// local vectors for .disp file data storage
	std::vector<int> status;
	std::vector<double> objmin;
	std::vector<Point> dis;			// point displacements

	// read .disp file and check for success
	DispRead disp;
	if (!disp.read_disp_file_generalized(fname_disp, data.labels, data.points, status, objmin, dis)) return 0;
	int ndisp_pts = data.points.size();

	// check that .disp file contains at least sw number of points
	if (ndisp_pts < ndp) {
		std::cout << std::endl << "-> .disp file must have at least sw number of points " << std::endl << std::endl;
		return 0;
	}

	//
	// now move on to the .sort file
	// two options here:
	//	1. establish neighbors by sorting the point cloud
	//	2. read in an existing .sort file

	// read option
	if (do_sort_read) {
		disp.read_sort_file_cst_sv(fname_sort_read,data.neigh);
		std::cout << "using sort file: " << fname_sort_read << std::endl;
	}
	int nsort_pts = data.neigh.size();

	// check if .disp and .sort have equal numbers of points, revert to a new sort if not
	if ( (do_sort_read) && (ndisp_pts != nsort_pts) ){
		do_sort_read = false;
		data.neigh = {};
		std::cout << "-> different number of points in .disp and .sort" << std::endl;
	}

	// sort option
	if (!do_sort_read) {
		std::cout << "-> creating a new sort file: " << fname_base + ".sort.csv" << std::endl;
		// legacy brute-force sort
		//data.sort_order_neighbors(data.points[0]);

		// kdtree sort
		data.sort_neighbors_kdtree(data.points[0]);
		data.write_sort_file(fname_base,data.neigh);
	}

	//
	// check for a 2d coordinate plane point cloud to manage strain calc
	//

	std::vector<double> x_pos = {};
	std::vector<double> y_pos = {};
	std::vector<double> z_pos = {};
	for (unsigned int i=0; i<data.points.size(); i++) {
		x_pos.emplace_back(data.points[i].x());
		y_pos.emplace_back(data.points[i].y());
		z_pos.emplace_back(data.points[i].z());
	}
	double min_x = *min_element(x_pos.begin(), x_pos.end());
	double max_x = *max_element(x_pos.begin(), x_pos.end());

	double min_y = *min_element(y_pos.begin(), y_pos.end());
	double max_y = *max_element(y_pos.begin(), y_pos.end());

	double min_z = *min_element(z_pos.begin(), z_pos.end());
	double max_z = *max_element(z_pos.begin(), z_pos.end());

	if ( fabs(max_x - min_x) < 0.001 ) {	// a fixed limit is tricky, but this would work for typical voxel coordinates
		yz_only = true;
	}
	if ( fabs(max_y - min_y) < 0.001 ) {
		zx_only = true;
	}
	if ( fabs(max_z - min_z) < 0.001 ) {
		xy_only = true;
	}

	// output here to catch flag setting as well
	if (yz_only) {std::cout << "using 2d yz coordinate plane analysis" << std::endl;}
	if (zx_only) {std::cout << "using 2d zx coordinate plane analysis" << std::endl;}
	if (xy_only) {std::cout << "using 2d xy coordinate plane analysis" << std::endl;}

	// planar section end

	// echo other run parameters and guide faulty command line arguments
	std::cout << "using strain window: " << ndp << std::endl;

	if (objt == strain.objt_min()) {
		std::cout << "using objmin thresh: " << objt << " (use only highly matched points)" << std::endl;
	} else if (objt == strain.objt_max()) {
		std::cout << "using objmin thresh: " << objt << " (use all convergent points)" << std::endl;
	} else {
		std::cout << "using objmin thresh: " << objt << " " << std::endl;
	}

	if (refill) {
		std::cout << "using strain window refill " << std::endl;
	}

	std::cout << "ndisp_pts =" << ndisp_pts << std::endl;

	/******************************************************************************/
	//
	// strain calculation begins here
	//

	// model used for fitting, 3D quadratic polynomial
	//    m  = c0
	//       + c1*x + c2*y + c3*z
	//       + c4*x^2 + c5*y^2 + c6*z^2
	//       + c7*x*y + c8*y*z + c9*x*z

	for (unsigned int i=0; i<data.points.size(); i++) {

		// get neighborhood indices for this point
		std::vector<int> nbr_ind = data.neigh[i];

		// find matrix scaling factor and load neighborhood vectors snpos and sndis for current point
		// using the distance to the current point (neighborhood center) for scaling
		Point origin(0.0, 0.0, 0.0);
		double distance = data.points[i].pt_dist(origin);
		double scale = 1.0/distance;

		// scale the calculation location (x,y,z in the polyfit)
		Point base(scale*data.points[i].x(), scale*data.points[i].y(), scale*data.points[i].z());

		// note: scaling is done because positions may be large numbers and get squared in the X matrix
		std::vector<Point> npos = {};	// neighbor positions admitted to the sw, use for evaluating sw voxel size
		std::vector<Point> snpos = {};	// scaled neighbor positions admitted to the sw
		std::vector<Point> sndis = {};	// scaled neighbor displacements admitted to the sw

		// establish neighborhoods and load neighbor position and displacement data
		// the for and while loop versions are essentially identical but the while loop looks for more good points up to sw setting
		// ndp is the requested number of neighborhood points for the strain windows
		// check nbr status, objmin threshold, and still within range of available nbr points
		
		if (!refill) {
			int indx = 0;
			for (unsigned int j=0; j<ndp; j++) {
				if ( (status[nbr_ind[indx]] == 0) && (objmin[nbr_ind[indx]] <= objt) && (indx < nbr_ind.size())) {
					Point pos(data.points[nbr_ind[indx]].x(), data.points[nbr_ind[indx]].y(), data.points[nbr_ind[indx]].z());
					Point spos(scale*data.points[nbr_ind[indx]].x(), scale*data.points[nbr_ind[indx]].y(), scale*data.points[nbr_ind[indx]].z());
					npos.push_back(pos);
					snpos.push_back(spos);
					Point sdis(scale*dis[nbr_ind[indx]].x(), scale*dis[nbr_ind[indx]].y(), scale*dis[nbr_ind[indx]].z());
					sndis.push_back(sdis);
				}
				indx += 1;	// generally would use j but the while loop version needs to count
			}
		} else {
			int indx = 0;
			int np = 0;
			while ( (np < ndp) && (indx < ndp_max) ) {
				if ( (status[nbr_ind[indx]] == 0) && (objmin[nbr_ind[indx]] <= objt) && (indx < nbr_ind.size())) {
					Point pos(data.points[nbr_ind[indx]].x(), data.points[nbr_ind[indx]].y(), data.points[nbr_ind[indx]].z());
					Point spos(scale*data.points[nbr_ind[indx]].x(), scale*data.points[nbr_ind[indx]].y(), scale*data.points[nbr_ind[indx]].z());
					npos.push_back(pos);
					snpos.push_back(spos);
					Point sdis(scale*dis[nbr_ind[indx]].x(), scale*dis[nbr_ind[indx]].y(), scale*dis[nbr_ind[indx]].z());
					sndis.push_back(sdis);
					np += 1;
				}
				indx += 1;
			}
		}

		// if no points in neigh then pts_in_sw becomes zero
		data.pts_in_sw.push_back(snpos.size());

		// determine sw_size in voxels as cube root of the bounding box volume
		// for planar analysis option set "thin" direction to 1.0
		double min_x = std::numeric_limits<double>::max();
		double max_x = std::numeric_limits<double>::min();
		double min_y = std::numeric_limits<double>::max();
		double max_y = std::numeric_limits<double>::min();
		double min_z = std::numeric_limits<double>::max();
		double max_z = std::numeric_limits<double>::min();

		for (unsigned int j=0; j<npos.size(); j++) {
			min_x = std::min(min_x, npos[j].x());  max_x = std::max(max_x, npos[j].x());
			min_y = std::min(min_y, npos[j].y());  max_y = std::max(max_y, npos[j].y());
			min_z = std::min(min_z, npos[j].z());  max_z = std::max(max_z, npos[j].z());
		}
		double diff_x = (max_x - min_x);
		double diff_y = (max_y - min_y);
		double diff_z = (max_z - min_z);
		double npos_box_vol = diff_x*diff_y*diff_z;
		double box_equiv_edge = std::cbrt(npos_box_vol);
		// planar cases
		double npos_box_area;
		if (yz_only) {
			npos_box_area = diff_y*diff_z;
			box_equiv_edge = std::sqrt(npos_box_area);
		}
		if (zx_only) {
			npos_box_area = diff_z*diff_x;
			box_equiv_edge = std::sqrt(npos_box_area);
		}
		if (xy_only) {
			npos_box_area = diff_x*diff_y;
			box_equiv_edge = std::sqrt(npos_box_area);
		}

		if (snpos.size() == 0) {
			data.sw_size.emplace_back(0.0);

		} else {
			data.sw_size.emplace_back(box_equiv_edge);
		}

		int ngp = snpos.size();		// number of good points, may be zero
		int nmp = strain.nmp();		// set by poly model choice, fixed in constructor
		int nsd = 3;				// number of spatial dimensions for calc

		// OLS variables sized to number of good points and reinitialized
		Eigen::MatrixXd dvs = Eigen::MatrixXd(ngp,nsd);		// for u,v,w fits, use .col for individual access
		Eigen::VectorXd rhs = Eigen::VectorXd(nmp);
		Eigen::MatrixXd Xm = Eigen::MatrixXd(ngp,nmp);
		Eigen::MatrixXd XtXm = Eigen::MatrixXd(nmp,nmp);
		Eigen::VectorXd par = Eigen::VectorXd(nmp);			// the polynomial parameters, c0->9

		Eigen::Matrix3d DG;		// deformation gradient storage 
		Eigen::Matrix3d ST;		// strain tensor storage 

		std::vector<double> lam(nsd);			// eigenvalues for a point
		std::vector<double> str(nsd*nsd);		// exx,eyy,ezz,exy,eyz,exz,p1,p2,p3 for a point, for strain pushback to data
		std::vector<double> dgd(nsd*nsd);		// ux,uy,uz,vx,vy,vz,wx,wy,wz for a point, for disp grad pushback to data
		std::vector<double> rot(2*nsd+1);		// rx,ry,rz,nx,ny,nz,deg - screw axis in radians and unit vector with degrees
		std::vector<double> displ(nsd);			// u,v,w calculated from the fit equations

		for (unsigned int j=0; j<ngp; j++) {	// not simply ndp, points may have different numbers of sw points

			Xm(j,0) = 1;

			Xm(j,1) = snpos[j].x();
			Xm(j,2) = snpos[j].y();
			Xm(j,3) = snpos[j].z();

			Xm(j,4) = snpos[j].x()*snpos[j].x();
			Xm(j,5) = snpos[j].y()*snpos[j].y();
			Xm(j,6) = snpos[j].z()*snpos[j].z();

			Xm(j,7) = snpos[j].x()*snpos[j].y();
			Xm(j,8) = snpos[j].y()*snpos[j].z();
			Xm(j,9) = snpos[j].x()*snpos[j].z();

			dvs(j,0) = sndis[j].x();
			dvs(j,1) = sndis[j].y();
			dvs(j,2) = sndis[j].z();

			// adjust for coordinate plane point clouds, auto detected or activated with -xy, -yz, -zx flags

			if (yz_only) {			// zero out x terms
				Xm(j,1) = 0.0;
				Xm(j,4) = 0.0;
				Xm(j,7) = 0.0;
				Xm(j,9) = 0.0;

				dvs(j,0) = 0.0;
			}

			if (zx_only) {			// zero out y terms
				Xm(j,2) = 0.0;
				Xm(j,5) = 0.0;
				Xm(j,7) = 0.0;
				Xm(j,8) = 0.0;

				dvs(j,1) = 0.0;
			}

			if (xy_only) {			// zero out z terms
				Xm(j,3) = 0.0;
				Xm(j,6) = 0.0;
				Xm(j,8) = 0.0;
				Xm(j,9) = 0.0;

				dvs(j,2) = 0.0;
			}

		}

		XtXm = Xm.transpose()*Xm;

		for (unsigned int j=0; j<3; j++) {
			rhs = Xm.transpose()*dvs.col(j);
			par = XtXm.fullPivHouseholderQr().solve(rhs);

			// deformation gradient
			DG(j,0) = par(1) + 2*par(4)*base.x() + par(7)*base.y() + par(9)*base.z();
			DG(j,1) = par(2) + 2*par(5)*base.y() + par(7)*base.x() + par(8)*base.z();
			DG(j,2) = par(3) + 2*par(6)*base.z() + par(8)*base.y() + par(9)*base.x();

			// local volume fit displacement values
			displ[j] = distance*( par(0) 
								+ par(1)*base.x() + par(2)*base.y() + par(3)*base.z() 
								+ par(4)*base.x()*base.x() + par(5)*base.y()*base.y() + par(6)*base.z()*base.z() 
								+ par(7)*base.x()*base.y() + par(8)*base.y()*base.z() + par(9)*base.x()*base.z() );
		}
		data.dis_vfit.push_back(displ);
		dgd = {DG(0,0),DG(0,1),DG(0,2),DG(1,0),DG(1,1),DG(1,2),DG(2,0),DG(2,1),DG(2,2)};
		data.DispGrd.push_back(dgd);


		// ***** introduce unified kinematic approach here
		// no specialized fiber/plane orientation output included yet

		// instantiate the Kinematics class
		const strain::Kinematics k(DG);

		// generate derived quantities
		PointResultBasic k_res;
		k_res.valid = k.valid();                    // false if det F <= 0: values below are NaN
        k_res.J = k.jacobian();
        k_res.volumetric = k.volumetricStrain();
        k_res.equivalent = k.equivalentStrain();
        k_res.principalStrain = k.principalStretches().array() - 1.0;
        k_res.biot = k.biot();
        k_res.henc = k.hencky();
        k_res.grla = k.greenLagrange();
        k_res.rvec = k.rotationVector();

		// these calculate the principal values for each strain tensor and return as a simple vector
		const Eigen::Array3d lambda = k.principalStretches().array();
		Eigen::Vector3d biotP = (lambda - 1.0).matrix();
		Eigen::Vector3d grlaP = (0.5 * (lambda.square() - 1.0)).matrix();
		Eigen::Vector3d hencP = lambda.log().matrix();

		// load into results vectors
		str = {k_res.biot(0,0),k_res.biot(1,1),k_res.biot(2,2),
				k_res.biot(0,1),k_res.biot(1,2),k_res.biot(0,2),
				 biotP[0],biotP[1],biotP[2]};
		data.Biot.push_back(str);

		str = {k_res.grla(0,0),k_res.grla(1,1),k_res.grla(2,2),
				k_res.grla(0,1),k_res.grla(1,2),k_res.grla(0,2),
				 grlaP[0],grlaP[1],grlaP[2]};
		data.GrLa.push_back(str);

		str = {k_res.henc(0,0),k_res.henc(1,1),k_res.henc(2,2),
				k_res.henc(0,1),k_res.henc(1,2),k_res.henc(0,2),
				 hencP[0],hencP[1],hencP[2]};
		data.Henc.push_back(str);
		
		// add to basic k.rotationVector() output a unit vector and a magnitude in degrees
		double rvec_mag_rad = k_res.rvec.norm();	// radians
		double rvec_mag_deg = rvec_mag_rad*(180.0/3.14159);	// degrees
		rot = {k_res.rvec(0),k_res.rvec(1),k_res.rvec(2),
				k_res.rvec(0)/rvec_mag_rad,k_res.rvec(1)/rvec_mag_rad,k_res.rvec(2)/rvec_mag_rad,
				  rvec_mag_deg};
		data.RotAxis.push_back(rot);

	}	// end data.points loop

	// output result files

	// Biot strain (new)
	if (out_Biot) {
		std::string fname = fname_base + "-sw" + std::to_string(ndp) + ".Biot.csv";
		std::string header = "n,x,y,z,u_fit,v_fit,w_fit,pts_in_sw,sw_size,exx,eyy,ezz,exy,eyz,exz,ep1,ep2,ep3";
		strain.write_output(fname, header, data, data.Biot);
	}

	// Lagrangian strain (new)
	if (out_GrLa) {
		std::string fname = fname_base + "-sw" + std::to_string(ndp) + ".GrLa.csv";
		std::string header = "n,x,y,z,u_fit,v_fit,w_fit,pts_in_sw,sw_size,exx,eyy,ezz,exy,eyz,exz,ep1,ep2,ep3";
		strain.write_output(fname, header, data, data.GrLa);
	}

	// Hencky (ln, true) strain (new) - note, integrate into command line parse
	if (out_Henc) {
		std::string fname = fname_base + "-sw" + std::to_string(ndp) + ".Henc.csv";
		std::string header = "n,x,y,z,u_fit,v_fit,w_fit,pts_in_sw,sw_size,exx,eyy,ezz,exy,eyz,exz,ep1,ep2,ep3";
		strain.write_output(fname, header, data, data.Henc);
	}

	// displacement gradient
	if (out_dgrd) {
		std::string fname = fname_base + "-sw" + std::to_string(ndp) + ".Dgrd.csv";
		std::string header = "n,x,y,z,u_fit,v_fit,w_fit,pts_in_sw,sw_size,ux,uy,uz,vx,vy,vz,wx,wy,wz";
		strain.write_output(fname, header, data, data.DispGrd);
	}

	// screw axis
	if (out_rvec) {
		std::string fname = fname_base + "-sw" + std::to_string(ndp) + ".Rvec.csv";
		std::string header = "n,x,y,z,u_fit,v_fit,w_fit,pts_in_sw,sw_size,rx,ry,rz,nx,ny,nz,deg";
		strain.write_output(fname, header, data, data.RotAxis);
	}

	std::cout << std::endl;
	return 0;
}
/******************************************************************************/
void StrainCalc::write_output(std::string fname, std::string hdr, DataCloud &data, std::vector<std::vector<double>> &output)
{
	std::ofstream outfile;
	outfile.open(fname);

	outfile << hdr << std::endl;

	for (unsigned int i=0; i<data.points.size(); i++)
	{
		outfile << data.labels[i];
		outfile << std::fixed << std::setprecision(2);
		outfile << "," << data.points[i].x() << "," << data.points[i].y() << "," << data.points[i].z();
		for (auto &j : data.dis_vfit[i]){outfile << "," << j;}
		outfile << "," << data.pts_in_sw[i];
		outfile << "," << std::fixed << std::setprecision(2) << data.sw_size[i];
		outfile << std::fixed << std::setprecision(6);
		for (auto &j : output[i]){outfile << "," << j;}		
		outfile << std::endl;
	}

	outfile.close();

	return;
}
/******************************************************************************/
// original strain calculaiton codes, superseeded by the generalized kinemtic approach

		/* within point loop
		// Engineering strain tensor

		ST = 0.5*(DG + DG.transpose());
		EigenSolver<MatrixXd> esE(ST);
		lam = {esE.eigenvalues()[0].real(), esE.eigenvalues()[1].real(), esE.eigenvalues()[2].real()};
		std::sort(lam.begin(), lam.end());
		std::reverse(lam.begin(), lam.end());
		// adjust for planar calcs, put the two principals into p1 and p2 spots, make p3 = 0.0
		if ( (xy_only) || (yz_only) || (zx_only)) {
			if (lam[0] == 0.0) {
				lam[0] = lam[1];
				lam[1] = lam[2];
				}
			if (lam[1] == 0.0) {
				lam[1] = lam[2];
			}
			lam[2] = 0.0;
		}
		str = {ST(0,0),ST(1,1),ST(2,2),ST(0,1),ST(1,2),ST(0,2),lam[0],lam[1],lam[2]};
		data.Estrain.push_back(str);

		// Lagrangian strain tensor

		ST = 0.5*(DG + DG.transpose() + DG.transpose()*DG);
		EigenSolver<MatrixXd> esL(ST);
		lam = {esL.eigenvalues()[0].real(), esL.eigenvalues()[1].real(), esL.eigenvalues()[2].real()};
		std::sort(lam.begin(), lam.end());
		std::reverse(lam.begin(), lam.end());
		// adjust for planar calcs, put the two principals into p1 and p2 spots, make p3 = 0.0
		if ( (xy_only) || (yz_only) || (zx_only)) {
			if (lam[0] == 0.0) {
				lam[0] = lam[1];
				lam[1] = lam[2];
				}
			if (lam[1] == 0.0) {
				lam[1] = lam[2];
			}
			lam[2] = 0.0;
		}
		str = {ST(0,0),ST(1,1),ST(2,2),ST(0,1),ST(1,2),ST(0,2),lam[0],lam[1],lam[2]};
		data.Lstrain.push_back(str);
		*/

	/* after point loop

	// Engineering strain (old)
	if (out_Estr) {
		std::string fname = fname_base + "-sw" + std::to_string(ndp) + ".Estr.csv";
		std::string header = "n,x,y,z,u_fit,v_fit,w_fit,pts_in_sw,sw_size,exx,eyy,ezz,exy,eyz,exz,ep1,ep2,ep3";
		strain.write_output(fname, header, data, data.Estrain);
	}

	// Lagrangian strain (old)
	if (out_Lstr) {
		std::string fname = fname_base + "-sw" + std::to_string(ndp) + ".Lstr.csv";
		std::string header = "n,x,y,z,u_fit,v_fit,w_fit,pts_in_sw,sw_size,exx,eyy,ezz,exy,eyz,exz,ep1,ep2,ep3";
		strain.write_output(fname, header, data, data.Lstrain);
	}
	*/


/******************************************************************************/
// old command line parsing, not used
/*
int StrainCalc::find_flag(std::string flag, int &argc, char *argv[])
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
*/
/******************************************************************************/
/*
int StrainCalc::find_flag(std::string flag, int &argc, char *argv[], int &val)
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
*/
/******************************************************************************/
/*
int StrainCalc::find_flag(std::string flag, int &argc, char *argv[], double &val)
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
*/
/******************************************************************************/
/*
int StrainCalc::find_flag(size_t pos, size_t len, std::string flag, int &argc, char *argv[])
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
*/
/******************************************************************************/