#include "strain.h"






/******************************************************************************/
StrainCalc::StrainCalc ()
{

	num_mod_para = 10;

	num_data_points_min = 10;		// min number for the strain window size
	num_data_points_max = 50;		// max number for the strain window size
	num_data_points_def = 25;		// default number for the strain window size

	objmin_thresh_min = 0.0001;		// only highly matched points used
	objmin_thresh_max = 1.0;		// all points used
	objmin_thresh_def = 1.0;		// default is no thresholding
	
}
/******************************************************************************/
int StrainCalc::find_flag(std::string flag, int &argc, char *argv[]) 
{

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
int StrainCalc::find_flag(std::string flag, int &argc, char *argv[], int &val) 
{

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

		// no next argument on command line
		std::cout << "no argument after " << flag << " flag" << std::endl;
		}
	}
	return 0;
}
/******************************************************************************/
int StrainCalc::find_flag(std::string flag, int &argc, char *argv[], double &val) 
{

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

		// no next argument on command line
		std::cout << "no argument after " << flag << " flag" << std::endl;
		}
	}
	return 0;
}
/******************************************************************************/
int main(int argc, char *argv[])
{
	std::cout << endl << "strain calculation from strain main" << endl;

	StrainCalc strain;

	int ndp_min = strain.ndp_min();	
	int ndp_max = strain.ndp_max();
	int ndp = strain.ndp_def();			// may be modified by command inputs

	double objt_min = strain.objt_min();
	double objt_max = strain.objt_max();
	double objt = strain.objt_def();	// may be modified by command inputs

	std::string fname_disp;
	std::string fname_base;
	std::string fname_sort_read;

	bool do_sort_read = false;
	bool refill = false;				// try to fill neighborhood up to ndp, replacing (-r) bad and high objmin points

	bool out_Estr = false;
	bool out_Lstr = true;
	bool out_dgrd = false;

	// command entered with no arguments
	if (argc == 1)

	// ** update for new argument read process

	{
		std::cout << std::endl;
		std::cout << "Strain command needs more input." << std::endl;
		std::cout << "Command line options:" << std::endl;
		std::cout << "strain dvc.disp.csv" << "\t\t\t" << "-> use dvc output file dvc.disp.csv, create sort file, use default strain window of " << strain.ndp_def() << std::endl;
		std::cout << "strain dvc.disp.csv 30" << "\t\t\t" << "-> use dvc output file dvc.disp.csv, create sort file, use strain window between " << strain.ndp_min() << " and " << strain.ndp_max() << std::endl;
		std::cout << "strain dvc.disp.csv dvc.sort.csv" << "\t" << "-> use dvc output file dvc.disp.csv and sort file dvc.sort.csv with default strain window" << std::endl;
		std::cout << "strain dvc.disp.csv dvc.sort.csv 25" << "\t" << "-> use dvc output file dvc.disp.csv and sort file dvc.sort.csv with specified strain window" << std::endl;
		std::cout << "Command line flags:" << std::endl;
		std::cout << "\t" << "-t 0.01" << "\t\t" <<  "-> threshold strain windows, only use points with objmin below specified value (between "  << strain.objt_min() << " and " << strain.objt_max() << ")" << std::endl;
		std::cout << "\t" << "-r" << "\t\t" <<  "-> refill strain window to replace points failing threshold (may expand strain window size)" << std::endl;
		std::cout << "\t" << "-E" << "\t\t" <<	"-> add engineering strain output (default is Lagrangian)" << std::endl;
		std::cout << "\t" << "-D" << "\t\t" <<	"-> add displacement gradient tensor output" << std::endl;
		std::cout << "\t" << "-A" << "\t\t" <<	"-> write all output files" << std::endl;
		std::cout << std::endl;
		return 0;
	}

	// ** seg fault if disp and sort file are swapped, or both disp or both sort, need to check file contents in some way

	// look for the flag then take it out and adjust argc and argv accordingly

	if (strain.find_flag("-sw", argc, argv, ndp)) {	// strain window number of points
		if (ndp < ndp_min) ndp = ndp_min;
		if (ndp > ndp_max) ndp = ndp_max;
	} 

	if (strain.find_flag("-t", argc, argv, objt)) { // objmin threshold
		if (objt < objt_min) objt = objt_min;
		if (objt > objt_max) objt = objt_max;	
	} 

	if (strain.find_flag("-r", argc, argv)) {		// use strain window refill option
		refill = true;
	}

	if (strain.find_flag("-E", argc, argv)) {
		out_Estr = true;
	}

	if (strain.find_flag("-D", argc, argv)) {
		out_dgrd = true;
	}

	if (strain.find_flag("-A", argc, argv)) {
		out_Estr = true;
		out_dgrd = true;
	}



	// process first command argument
	if (argc > 1) {
		// check for .disp file
		fname_disp = argv[1];
		std::ifstream input_disp;
		input_disp.open(fname_disp.c_str());
		if (!input_disp.good())
		{
			std::cout << "\nCannot find .disp file '" << fname_disp << "'\n\n" ;
			return 0;
		}
		input_disp.close();
	}

	// assuming disp file name has no extension or one or more .extensions
	// work with the name up to the first extension to form output file names

	std::cout << "using disp file: " << fname_disp << std::endl;

	size_t fname_disp_length = fname_disp.length();
	size_t fname_disp_to_dot = fname_disp.find_first_of(".");

	if (fname_disp_to_dot == string::npos) {	// no dots in file name
		fname_base = fname_disp;
	} else {
		fname_base = fname_disp;
		fname_base.erase(fname_base.end() - (fname_disp_length - fname_disp_to_dot), fname_base.end());
	}

	//	std::cout << "fname_base = " << fname_base << std::endl;

	// command entered with two arguments
	// argv[2] could be a .sort filename or a window spec
	// ? check for same file name as .disp but with .sort and try that one?
	if (argc >= 3)
	{
		// check for sort file to read
		fname_sort_read = argv[2];
		std::ifstream input_sort_read;
		input_sort_read.open(fname_sort_read.c_str());
		if (input_sort_read.good())
		{			
			do_sort_read = true;
		} else {
			do_sort_read = false;
		}
		input_sort_read.close();
	}

	// prepare local vectors and instantiate a data cloud for .disp file data storage
	std::vector<int> status;
	std::vector<double> objmin;
	std::vector<Point> dis;
	DataCloud data;

	// read .disp file and check for success, read_disp_file prints error message
	DispRead disp;
//	if (!disp.read_disp_file(fname_disp,data.labels,data.points,status,objmin,dis)) return 0;
	if (!disp.read_disp_file_cst_sv(fname_disp,data.labels,data.points,status,objmin,dis)) return 0;
	int ndisp_pts = data.points.size();


//	return 0;



	// two options here:
	//	1. establish neighbors by sorting the point cloud (done)
	//	2. read in an existing .sort file 
	// 		sorted_pc_file.open(fname + ".sort");

	// read option
	if (do_sort_read) { 
	//	disp.read_sort_file(fname_sort_read,data.neigh);
		disp.read_sort_file_cst_sv(fname_sort_read,data.neigh);
		std::cout << "using sort file: " << fname_sort_read << std::endl;
	}
	int nsort_pts = data.neigh.size();

	std::cout << "nsort_pts =" << nsort_pts << std::endl;
	std::cout << "ndisp_pts =" << ndisp_pts << std::endl;

	// check if .disp and .sort have equal numbers of points, revert to a new sort if not
	if ( (do_sort_read) && (ndisp_pts != nsort_pts) ){
		do_sort_read = false;
		data.neigh = {};
		std::cout << "-> different number of points in .disp and .sort" << std::endl;
	}

	// sort option
	if (!do_sort_read) {
		std::cout << "-> creating a new sort file: " << fname_base + ".sort.csv" << std::endl;
		data.sort_order_neighbors(ndp_max);
		data.write_sort_file(fname_base,data.neigh);
	}

	// echo other run parameters and guide faulty commend line arguments
	std::cout << "using strain window: " << ndp << std::endl;

	if (objt == strain.objt_min()) {
		std::cout << "using objmin thresh: " << objt << " (use only highly matched points)" << std::endl;
	} else if (objt == strain.objt_max()) {
		std::cout << "using objmin thresh: " << objt << " (use all points)" << std::endl;
	} else {
		std::cout << "using objmin thresh: " << objt << " " << std::endl;
	}

	if (refill) {
		std::cout << "using strain window refill " << std::endl;
	}


	// *** StrainCalc strain;

	// model used for fitting, 3D quadratic polynomial
	//    m  = c0 
	//       + c1*x + c2*y + c3*z
	//       + c4*x^2 + c5*y^2 + c6*z^2 
	//       + c7*x*y + c8*y*z + c9*x*z

	std::vector<Eigen::MatrixXd> DG_all_pts = {};	// store for output

	for (unsigned int i=0; i<data.points.size(); i++) {

		// get neighborhood indices for this point
		std::vector<int> nbr_ind = data.neigh[i];

		// find matrix scaling factor and load neighborhood vectors npos and ndis for current point
		// using the distance to the current point (neighborhood center) for scaling 
		Point origin(0.0, 0.0, 0.0);
		double distance = data.points[i].pt_dist(origin);
		double scale = 1.0/distance;

		// scale the calculation location (x,y,z in the polyfit)
		Point base(scale*data.points[i].x(), scale*data.points[i].y(), scale*data.points[i].z());

		std::vector<Point> npos = {};
		std::vector<Point> ndis = {};

		// establish neighborhoods and load neighbor position and displacement data

		
		if (!refill) {
			int indx = 0;
			for (unsigned int j=0; j<ndp; j++) {
				if ( (status[nbr_ind[indx]] == 0) && (objmin[nbr_ind[indx]] <= objt) ) {			
					Point spos(scale*data.points[nbr_ind[indx]].x(), scale*data.points[nbr_ind[indx]].y(), scale*data.points[nbr_ind[indx]].z());
					npos.push_back(spos);
					Point sdis(scale*dis[nbr_ind[indx]].x(), scale*dis[nbr_ind[indx]].y(), scale*dis[nbr_ind[indx]].z());
					ndis.push_back(sdis);				
				}
				indx += 1;	// generally would use j but the while loop version needs to count
			}
		} else {
			int indx = 0;
			int np = 0;
			while ( (np < ndp) && (indx < ndp_max) ) {
				if ( (status[nbr_ind[indx]] == 0) && (objmin[nbr_ind[indx]] <= objt) ) {			
					Point spos(scale*data.points[nbr_ind[indx]].x(), scale*data.points[nbr_ind[indx]].y(), scale*data.points[nbr_ind[indx]].z());
					npos.push_back(spos);
					Point sdis(scale*dis[nbr_ind[indx]].x(), scale*dis[nbr_ind[indx]].y(), scale*dis[nbr_ind[indx]].z());
					ndis.push_back(sdis);		
					np += 1;		
				}
				indx += 1;
			}
		}


		// if no points in neigh then pts_in_sw becomes zero
		data.pts_in_sw.push_back(npos.size());
 
		if (npos.size() == 0) {
			data.sw_rad.emplace_back(0.0);

		} else {
			data.sw_rad.emplace_back(distance*base.pt_dist(npos.back()));
		}

		int ngp = npos.size();		// number of good points, may be zero
		int nmp = strain.nmp();				// set by poly model choice, fixed in constructor

		// OLS variables sized to number of good points and reinitialized
		Eigen::MatrixXd dvs = Eigen::MatrixXd(ngp,3);		// for u,v,w fits, use .col for individual access
		Eigen::VectorXd rhs = Eigen::VectorXd(nmp);
		Eigen::MatrixXd Xm = Eigen::MatrixXd(ngp,nmp);
		Eigen::MatrixXd XtXm = Eigen::MatrixXd(nmp,nmp);
		Eigen::VectorXd par = Eigen::VectorXd(nmp);			// the polynomial parameters, c0->9

		Eigen::MatrixXd DG = Eigen::MatrixXd(3,3);		// deformation gradient
		Eigen::MatrixXd ST = Eigen::MatrixXd(3,3);		// strain tensor storage

		std::vector<double> lam(3);			// eigenvalues for a point
		std::vector<double> str(9);			// exx,eyy,ezz,exy,eyz,exz,p1,p2,p3 for a point, for pushback to data
		std::vector<double> displ(3);		// u,v,w calculated from the fit equations

		for (unsigned int j=0; j<ngp; j++) {	// not simply ndp, points may have different numbers of sw points
			Xm(j,0) = 1;

			Xm(j,1) = npos[j].x();
			Xm(j,2) = npos[j].y();
			Xm(j,3) = npos[j].z();

			Xm(j,4) = npos[j].x()*npos[j].x();
			Xm(j,5) = npos[j].y()*npos[j].y();
			Xm(j,6) = npos[j].z()*npos[j].z();

			Xm(j,7) = npos[j].x()*npos[j].y();
			Xm(j,8) = npos[j].y()*npos[j].z();
			Xm(j,9) = npos[j].x()*npos[j].z();

			dvs(j,0) = ndis[j].x();
			dvs(j,1) = ndis[j].y();
			dvs(j,2) = ndis[j].z();
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
			displ[j] = distance*( par(0) \
								+ par(1)*base.x() + par(2)*base.y() + par(3)*base.z() \
								+ par(4)*base.x()*base.x() + par(5)*base.y()*base.y() + par(6)*base.z()*base.z() \
								+ par(7)*base.x()*base.y() + par(8)*base.y()*base.z() + par(9)*base.x()*base.z() );
		}
		data.dis_vfit.push_back(displ);

		DG_all_pts.emplace_back(DG);

		// Engineering strain tensor

		ST = 0.5*(DG + DG.transpose());

		EigenSolver<MatrixXd> esE(ST);

		lam = {esE.eigenvalues()[0].real(), esE.eigenvalues()[1].real(), esE.eigenvalues()[2].real()};
		std::sort(lam.begin(), lam.end());
		std::reverse(lam.begin(), lam.end());
		str = {ST(0,0),ST(1,1),ST(2,2),ST(0,1),ST(1,2),ST(0,2),lam[0],lam[1],lam[2]};
		data.Estrain.push_back(str);

		// Lagrangian strain tensor

		ST = 0.5*(DG + DG.transpose() + DG.transpose()*DG);

		EigenSolver<MatrixXd> esL(ST);

		lam = {esL.eigenvalues()[0].real(), esL.eigenvalues()[1].real(), esL.eigenvalues()[2].real()};
		std::sort(lam.begin(), lam.end());
		std::reverse(lam.begin(), lam.end());
		str = {ST(0,0),ST(1,1),ST(2,2),ST(0,1),ST(1,2),ST(0,2),lam[0],lam[1],lam[2]};
		data.Lstrain.push_back(str);
	}

	// output result files

	std::string hdr = "n,x,y,z,u_fit,v_fit,w_fit,pts_in_sw,sw_radius,exx,eyy,ezz,exy,eyz,exz,ep1,ep2,ep3";
	std::string of_name;

	// Engineering strain
	if (out_Estr) {
		std::ofstream strain_results_E;
		of_name = fname_base + "-sw" + std::to_string(ndp) + ".Estr.csv";
		std::cout << "output file: " << of_name << std::endl;
		strain_results_E.open(of_name);
		strain_results_E << hdr << std::endl;
		for (unsigned int i=0; i<data.points.size(); i++)
		{
			strain_results_E << data.labels[i] << "," << data.points[i].x() << "," << data.points[i].y() << "," << data.points[i].z();
			for (auto &j : data.dis_vfit[i]){strain_results_E << "," << j;}
			strain_results_E << "," << data.pts_in_sw[i];
			strain_results_E << "," << data.sw_rad[i];
			for (auto &j : data.Estrain[i]){strain_results_E << "," << j;}
			strain_results_E << std::endl;
		}
		strain_results_E.close();
	}

	// Lagrangian strain
	if (out_Lstr) {
		std::ofstream strain_results_L;
		of_name = fname_base + "-sw" + std::to_string(ndp) + ".Lstr.csv";
		std::cout << "output file: " << of_name << std::endl;
		strain_results_L.open(of_name);
		strain_results_L << hdr << std::endl;
		for (unsigned int i=0; i<data.points.size(); i++)
		{
			strain_results_L << data.labels[i] << "," << data.points[i].x() << "," << data.points[i].y() << "," << data.points[i].z();
			for (auto &j : data.dis_vfit[i]){strain_results_L << "," << j;}
			strain_results_L << "," << data.pts_in_sw[i];
			strain_results_L << "," << data.sw_rad[i];
			for (auto &j : data.Lstrain[i]){strain_results_L << "," << j;}
			strain_results_L << std::endl;
		}
		strain_results_L.close();
	}


	// displacement gradient
	if (out_dgrd) {
		std::ofstream dispgrad_results;
		of_name = fname_base + "-sw" + std::to_string(ndp) + ".dgrd.csv";
		std::cout << "output file: " << of_name << std::endl;
		dispgrad_results.open(of_name);
		dispgrad_results << "n,x,y,z,u_fit,v_fit,w_fit,pts_in_sw,sw_radius,ux,uy,uz,vx,vy,vz,wx,wy,wz" << std::endl;
		for (unsigned int i=0; i<data.points.size(); i++) {
			dispgrad_results << data.labels[i] << "," << data.points[i].x() << "," << data.points[i].y() << "," << data.points[i].z();
			for (auto &j : data.dis_vfit[i]){dispgrad_results << "," << j;}
			dispgrad_results << "," << data.pts_in_sw[i];
			dispgrad_results << "," << data.sw_rad[i];

			for (unsigned int j=0; j<3; j++)
				for (unsigned int k=0; k<3; k++)
					dispgrad_results << "," << DG_all_pts[i](j,k);

			dispgrad_results << std::endl;
		}
		dispgrad_results.close();
	}


	return 0;
}
/******************************************************************************/
