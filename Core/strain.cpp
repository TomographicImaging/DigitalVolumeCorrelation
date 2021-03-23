#include "strain.h"






/******************************************************************************/
StrainCalc::StrainCalc ()
{

	num_mod_para = 10;

//	num_data_pts = 5;	// testing, straight line fit
	
}
/******************************************************************************/





/******************************************************************************/
int main(int argc, char *argv[])
{
	// Working, but still to do:
	//	-> set up argc and argv for proper inputs, no inputs, etc. (started)
	//	-> manage bad points in the .disp file
	//		-> does read automatically bypass bad points?
	//	-> do conditioning checks and trap poor OLS points
	//	-> formalize outputs file with headers (done)
	//	-> add strain results to DataCloud for potential visualization within gui (done)
	//	-> manage output: E? L? principal values? principal vectors? user selects?

	// can the strain command be issued out of the gui with the same command line argument style?

	std::cout << endl << "strain calculation from strain main" << endl;

	// this needs to go in constructor and coordinated with sorting routine in DataCloud
	// manage number of points (ndp) in the strain window for the OLS fit
	int ndp_min = 25;
	int ndp_max = 50;
	int ndp_default = 25;
	int ndp = ndp_default;		// may be modified by command inputs
	std::string fname_disp;

	// command entered with no arguments
	if (argc == 1)
	{
		std::cout << std::endl;
		std::cout << "Strain command needs more input." << std::endl;
		std::cout << "Options:" << std::endl;
		std::cout << "strain dvc.disp" << "\t\t\t" << "( .disp file specified, run with sorting and default strain window of " << ndp_default << ")" << std::endl;
		std::cout << "strain dvc.disp 30" << "\t\t" << "( .disp file specified, run with sorting and specified strain window between " << ndp_min << " and " << ndp_max << ")" << std::endl;
//		std::cout << "strain dvc.disp dvc.sort" << "\t" << "(identify .disp file, use existing .sort file and default strain window)" << std::endl;
//		std::cout << "strain dvc.disp dvc.sort 25" << "\t" << "(identify .disp file, use existing .sort file and specified strain window)" << std::endl;
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
	std::cout << "using displcement file: " << fname_disp << std::endl;

	// command entered with two arguments
	if (argc == 3)
	{
		// check validity of strain window spec
		try {
			ndp = std::stoi(argv[2]);
		}
		catch (const std::invalid_argument& ia) {
			std::cout << "-> Need an integer for the strain window size.\n";
			return 0;
		}

		if (ndp < ndp_min) ndp = ndp_min;
		if (ndp > ndp_max) ndp = ndp_max;
	} 
	std::cout << "using strain window: " << ndp << std::endl;

	// prepare local vectors and instantiate a data cloud for .disp file data storage
	std::vector<int> status;
	std::vector<double> objmin;
	std::vector<Point> dis;
	DataCloud data;

	// read .disp file and check for success, read_disp_file prints error message
	DispRead disp;
	if (!disp.read_disp_file(fname_disp,data.labels,data.points,status,objmin,dis)) return 0;

	// two options here:
	//	1. establish neighbors by sorting the point cloud (done)
	//	2. read in an existing .sort file (-> needs writing)
	// 		sorted_pc_file.open(fname + ".sort");


	data.sort_order_neighbors(ndp_max);
	data.write_sort_file(fname_disp,data.neigh);

	//***************

	// set up for OLS

	StrainCalc strain;

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

		// scale the derivative calculation location
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

			DG(j,0) = par(1) + 2*par(4)*spos.x() + par(7)*spos.y() + par(9)*spos.z();
			DG(j,1) = par(2) + 2*par(5)*spos.y() + par(7)*spos.x() + par(8)*spos.z();
			DG(j,2) = par(3) + 2*par(6)*spos.z() + par(8)*spos.y() + par(9)*spos.x();
		}

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

	std::string hdr = "n\tx\ty\tz\texx\teyy\tezz\texy\teyz\texz\tep1\tep2\tep3\n";

	std::ofstream strain_results_E;
	strain_results_E.open(fname_disp + ".Estrain");
	strain_results_E << hdr;
	for (unsigned int i=0; i<data.points.size(); i++)
	{
		strain_results_E << data.labels[i] << "\t" << data.points[i].x() << "\t" << data.points[i].y() << "\t" << data.points[i].z() << "\t";
		for (auto &j : data.Estrain[i]){strain_results_E << j << "\t";}
		strain_results_E << std::endl;
	}
	strain_results_E.close();

	std::ofstream strain_results_L;
	strain_results_L.open(fname_disp + ".Lstrain");
	strain_results_L << hdr;
	for (unsigned int i=0; i<data.points.size(); i++)
	{
		strain_results_L << data.labels[i] << "\t" << data.points[i].x() << "\t" << data.points[i].y() << "\t" << data.points[i].z() << "\t";
		for (auto &j : data.Lstrain[i]){strain_results_L << j << "\t";}
		strain_results_L << std::endl;
	}
	strain_results_L.close();

	return 0;
}
/******************************************************************************/
