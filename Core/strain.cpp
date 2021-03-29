#include "strain.h"






/******************************************************************************/
StrainCalc::StrainCalc ()
{

	num_mod_para = 10;

	num_data_points_min = 10;		// min number for the strain window size
	num_data_points_max = 50;		// max number for the strain window size
	num_data_points_def = 25;		// default number for the strain window size

	
}
/******************************************************************************/





/******************************************************************************/
int main(int argc, char *argv[])
{
	std::cout << endl << "strain calculation from strain main" << endl;

	StrainCalc strain;

	int ndp_min = strain.ndp_min();	
	int ndp_max = strain.ndp_max();
	int ndp = strain.ndp_def();		// may be modified by command inputs
	std::string fname_disp;
	std::string fname_base;
	std::string fname_sort_read;

	bool do_sort_read = false;

	// command entered with no arguments
	if (argc == 1)
	{
		std::cout << std::endl;
		std::cout << "Strain command needs more input." << std::endl;
		std::cout << "Options:" << std::endl;
		std::cout << "strain dvc.disp" << "\t\t\t" << "( .disp file specified, run with default strain window of " << strain.ndp_def() << ")" << std::endl;
		std::cout << "strain dvc.disp 30" << "\t\t" << "( .disp file specified, run with strain window between " << strain.ndp_min() << " and " << strain.ndp_max() << ")" << std::endl;
		std::cout << "strain dvc.disp dvc.sort" << "\t" << "(identify .disp file, use existing .sort file and default strain window)" << std::endl;
		std::cout << "strain dvc.disp dvc.sort 25" << "\t" << "(identify .disp file, use existing .sort file and specified strain window)" << std::endl;
		std::cout << std::endl;
		return 0;
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
	std::cout << "using disp file: " << fname_disp << std::endl;
	fname_base = fname_disp;
	fname_base.erase(fname_base.end()-5, fname_base.end());

	// command entered with two arguments
	// argv[2] could be a .sort filename or a window spec
	if (argc >= 3)
	{
		// check for sort file to read
		fname_sort_read = argv[2];
		std::ifstream input_sort_read;
		input_sort_read.open(fname_sort_read.c_str());
		if (input_sort_read.good())
		{
			std::cout << "using sort file: " << fname_sort_read << std::endl;			
			do_sort_read = true;
		} else {
			do_sort_read = false;
		}
		input_sort_read.close();

		// check for a valid strain window spec in either argc[2]
		try {
			ndp = std::stoi(argv[2]);
		}
		catch (const std::invalid_argument& ia) {
		}
		if (ndp < ndp_min) ndp = ndp_min;
		if (ndp > ndp_max) ndp = ndp_max;
	} 

	if (argc == 4)
	{
		try {
			ndp = std::stoi(argv[3]);
		}
		catch (const std::invalid_argument& ia) {
			std::cout << "-> put .sort file before the strain window size" << std::endl;
		}
		if (ndp < ndp_min) ndp = ndp_min;
		if (ndp > ndp_max) ndp = ndp_max;
	}

	if (!do_sort_read) {std::cout << "sorting disp file" << std::endl;}
	std::cout << "using strain window: " << ndp << std::endl;

	// prepare local vectors and instantiate a data cloud for .disp file data storage
	std::vector<int> status;
	std::vector<double> objmin;
	std::vector<Point> dis;
	DataCloud data;

	// read .disp file and check for success, read_disp_file prints error message
	DispRead disp;
	if (!disp.read_disp_file(fname_disp,data.labels,data.points,status,objmin,dis)) return 0;
	int ndisp_pts = data.points.size();

	// two options here:
	//	1. establish neighbors by sorting the point cloud (done)
	//	2. read in an existing .sort file 
	// 		sorted_pc_file.open(fname + ".sort");

	// read option
	if (do_sort_read) { 
		disp.read_sort_file(fname_sort_read,data.neigh);
	}
	int nsort_pts = data.neigh.size();

	// check if .disp and .sort have equal numbers of points, revert to a new sort if not
	if (ndisp_pts != nsort_pts) {
		do_sort_read = false;
		data.neigh = {};
		std::cout << "-> different number of points in .disp and .sort" << std::endl;
		std::cout << "-> creating a new sort file: " << fname_base + ".sort" << std::endl;
	}

	// sort option
	if (!do_sort_read) {
		data.sort_order_neighbors(ndp_max);
		data.write_sort_file(fname_base,data.neigh);
	}

	//***************

	// set up for OLS

	// StrainCalc strain;

	int nmp = strain.nmp();				// set by poly model choice, fixed in constructor
	int npts = data.points.size();		// total number of points in the .disp file

	// OLS variables
	Eigen::MatrixXd dvs = Eigen::MatrixXd(ndp,3);		// for u,v,w fits, use .col for individual access
	Eigen::VectorXd rhs = Eigen::VectorXd(nmp);
	Eigen::MatrixXd Xm = Eigen::MatrixXd(ndp,nmp);
	Eigen::MatrixXd XtXm = Eigen::MatrixXd(nmp,nmp);
	Eigen::VectorXd par = Eigen::VectorXd(nmp);			// the polynomial parameters, c0->9

	Eigen::MatrixXd DG = Eigen::MatrixXd(3,3);		// deformation gradient
	Eigen::MatrixXd ST = Eigen::MatrixXd(3,3);		// strain tensor storage

	std::vector<double> lam(3);		// eigenvalues for a point
	std::vector<double> str(9);		// exx,eyy,ezz,exy,eyz,exz,p1,p2,p3 for a point, for pushback to data
	std::vector<double> displ(3);		// u,v,w calculated from the fit equations

	// model used for fitting, 3D quadratic polynomial
	//    m  = c0 
	//       + c1*x + c2*y + c3*z
	//       + c4*x^2 + c5*y^2 + c6*z^2 
	//       + c7*x*y + c8*y*z + c9*x*z

	// loop for all points in the cloud

	for (unsigned int i=0; i<npts; i++) {

		// get neighborhood indices for this point
		std::vector<int> nbr_ind = data.neigh[i];

		// find matrix scaling factor and load neighborhood vectors npos and ndis for current point
		// using the distance to the current point (neighborhood center) for scaling 
		Point origin(0.0, 0.0, 0.0);
		double distance =  data.points[i].pt_dist(origin);
		double scale = 1.0/distance;

		std::vector<Point> npos = {};
		std::vector<Point> ndis = {};

		// load neighborhood data vectors from full list
		for (unsigned int j=0; j<ndp; j++) {
			Point spos(scale*data.points[nbr_ind[j]].x(), scale*data.points[nbr_ind[j]].y(), scale*data.points[nbr_ind[j]].z());
			npos.push_back(spos);
			Point sdis(scale*dis[nbr_ind[j]].x(), scale*dis[nbr_ind[j]].y(), scale*dis[nbr_ind[j]].z());
			ndis.push_back(sdis);
		}

		// scale the calculation location (x,y,z in the polyfit)
		Point spos(scale*data.points[i].x(), scale*data.points[i].y(), scale*data.points[i].z());

		for (unsigned int j=0; j<ndp; j++) {
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
			DG(j,0) = par(1) + 2*par(4)*spos.x() + par(7)*spos.y() + par(9)*spos.z();
			DG(j,1) = par(2) + 2*par(5)*spos.y() + par(7)*spos.x() + par(8)*spos.z();
			DG(j,2) = par(3) + 2*par(6)*spos.z() + par(8)*spos.y() + par(9)*spos.x();

			// local volume fit displacement values
			displ[j] = distance*( par(0) \
								+ par(1)*spos.x() + par(2)*spos.y() + par(3)*spos.z() \
								+ par(4)*spos.x()*spos.x() + par(5)*spos.y()*spos.y() + par(6)*spos.z()*spos.z() \
								+ par(7)*spos.x()*spos.y() + par(8)*spos.y()*spos.z() + par(9)*spos.x()*spos.z() );
		}
		data.dis_vfit.push_back(displ);

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

	std::string hdr = "n\tx\ty\tz\tu_fit\tv_fit\tw_fit\texx\teyy\tezz\texy\teyz\texz\tep1\tep2\tep3\n";
	std::string of_name;

	// Engineering strain
	std::ofstream strain_results_E;
	of_name = fname_base + "-sw" + std::to_string(ndp) + ".Estr";
	strain_results_E.open(of_name);
	strain_results_E << hdr;
	for (unsigned int i=0; i<data.points.size(); i++)
	{
		strain_results_E << data.labels[i] << "\t" << data.points[i].x() << "\t" << data.points[i].y() << "\t" << data.points[i].z() << "\t";
		for (auto &j : data.dis_vfit[i]){strain_results_E << j << "\t";}
		for (auto &j : data.Estrain[i]){strain_results_E << j << "\t";}
		strain_results_E << std::endl;
	}
	strain_results_E.close();

	// Lagrangian strain
	std::ofstream strain_results_L;
	of_name = fname_base + "-sw" + std::to_string(ndp) + ".Lstr";
	strain_results_L.open(of_name);
	strain_results_L << hdr;
	for (unsigned int i=0; i<data.points.size(); i++)
	{
		strain_results_L << data.labels[i] << "\t" << data.points[i].x() << "\t" << data.points[i].y() << "\t" << data.points[i].z() << "\t";
		for (auto &j : data.dis_vfit[i]){strain_results_L << j << "\t";}
		for (auto &j : data.Lstrain[i]){strain_results_L << j << "\t";}
		strain_results_L << std::endl;
	}
	strain_results_L.close();

	/*
	// Volume fit displacements output as standalone file
	std::ofstream displacements;
	hdr = "n\tx\ty\tz\tu_fit\tv_fit\tw_fit\n";
	of_name = fname_base + "-sw" + std::to_string(ndp) + ".vfit";
	displacements.open(of_name);
	displacements << hdr;
	for (unsigned int i=0; i<data.points.size(); i++)
	{
		displacements << data.labels[i] << "\t" << data.points[i].x() << "\t" << data.points[i].y() << "\t" << data.points[i].z() << "\t";
		for (auto &j : data.dis_vfit[i]){displacements << j << "\t";}
		displacements << std::endl;
	}	
	displacements.close();
	*/

	return 0;
}
/******************************************************************************/
