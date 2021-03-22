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

	std::cout << endl << "strain calculation from strain main" << endl;

	// Working, but still to do:
	//	-> set up argc and argv for proper inputs, no inputs, etc.
	//	-> manage bad points in the .disp file
	//		-> does read automatically bypass bad points?
	//	-> do conditioning checks and trap poor OLS points
	//	-> formalize outputs file with headers
	//	-> manage output: E? L? principal values? principal vectors? user selects?

	// read .disp file into vectors
	//std::vector<int> label;
	//std::vector<Point> pos;
	std::vector<int> status;
	std::vector<double> objmin;
	std::vector<Point> dis;

	// create a data cloud to store and sort points from the .disp file
	// -> expand DataCloud to manage these results

	DataCloud data;

	std::string fname;					// -> example file, add to command line 
	fname = "lava_cloud_full_cyl_inc_15.disp";

	DispRead disp;
	
	disp.read_disp_file(fname,data.labels,data.points,status,objmin,dis);

	// two options here:
	//	1. establish neighbors by sorting the point cloud (done)
	//	2. read in an existing .sort file (-> needs writing)

	int nnbr = 50;
	data.sort_order_neighbors(nnbr);
	data.write_sort_file("lava_cloud_full_cyl_inc_15.disp",data.neigh);

	//***************

	// set up for OLS
	StrainCalc strain;

	int nmp = strain.nmp();		// set by poly model choice, fixed in constructor

	// key parameter ... number of neighborhood points used for the strianv calc
	// needs to be greater than some min, 10 parameters but likely need about 25 points at least
	// max is the number of points used for sorting, nnbr, 50 currently
	int ndp = 25;				// user set, between a min and max set in constructor? 
	int npts = data.points.size();		// total number of points in the .disp file

	std::cout << endl << "nmp local = " << nmp << endl;
	std::cout << endl << "ndp local = " << ndp << endl;
	std::cout << endl << "npts local = " << npts << endl;	

	// OLS variables
	Eigen::MatrixXd dvs = Eigen::MatrixXd(ndp,3);	// for u,v,w fits
	Eigen::VectorXd rhs = Eigen::VectorXd(nmp);
	Eigen::MatrixXd Xm = Eigen::MatrixXd(ndp,nmp);
	Eigen::MatrixXd XtXm = Eigen::MatrixXd(nmp,nmp);
	Eigen::VectorXd par = Eigen::VectorXd(nmp);

	Eigen::MatrixXd D = Eigen::MatrixXd(3,3);		// deformation gradient
	Eigen::MatrixXd E = Eigen::MatrixXd(3,3);		// Engineering strain
	Eigen::MatrixXd L = Eigen::MatrixXd(3,3);		// Lagrangian strain

	// model used for fitting, 3D quadratic polynomial
	//    m  = c1 
	//       + c2*x + c3*y + c4*z
	//       + c5*x^2 + c6*y^2 + c7*z^2 
	//       + c8*x*y + c9*y*z + c10*x*z

	// output files for Eningeering and Lagrangian strain data
	std::ofstream strain_results_E;
	strain_results_E.open("lava_cloud_full_cyl_inc_15_E.strain");

	std::ofstream strain_results_L;
	strain_results_L.open("lava_cloud_full_cyl_inc_15_L.strain");

	// print headers
	strain_results_E << "n" << "\t" << "x" << "\t" << "y" << "\t" << "z" << "\t" << "exx" << "\t" << "eyy" << "\t" << "ezz" << "\t" << "exy" << "\t" << "eyz" << "\t" << "exz" << "\t" << "ep1" << "\t" << "ep2" << "\t" << "ep3" << std::endl;
	strain_results_L << "n" << "\t" << "x" << "\t" << "y" << "\t" << "z" << "\t" << "exx" << "\t" << "eyy" << "\t" << "ezz" << "\t" << "exy" << "\t" << "eyz" << "\t" << "exz" << "\t" << "ep1" << "\t" << "ep2" << "\t" << "ep3" << std::endl;

	for (unsigned int i=0; i<npts; i++) {
	//for (unsigned int i=0; i<1; i++) {

		// get neighborhood indices fro this point
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

			D(j,0) = par(1) + 2*par(4)*spos.x() + par(7)*spos.y() + par(9)*spos.z();
			D(j,1) = par(2) + 2*par(5)*spos.y() + par(7)*spos.x() + par(8)*spos.z();
			D(j,2) = par(3) + 2*par(6)*spos.z() + par(8)*spos.y() + par(9)*spos.x();
		}

		E = 0.5*(D + D.transpose());
		
		EigenSolver<MatrixXd> esE(E);
		std::vector<double> lamE = {esE.eigenvalues()[0].real(), esE.eigenvalues()[1].real(), esE.eigenvalues()[2].real()};
		std::sort(lamE.begin(), lamE.end());
		std::reverse(lamE.begin(), lamE.end());

		strain_results_E << data.labels[i] << "\t" << data.points[i].x() << "\t" << data.points[i].y() << "\t" << data.points[i].z() << "\t" << E(0,0) << "\t" << E(1,1) << "\t" << E(2,2) << "\t" << E(0,1) << "\t" << E(1,2) << "\t" << E(0,2) << "\t" << lamE[0] << "\t" << lamE[1] << "\t" << lamE[2] << std::endl;
		
		L = 0.5*(D + D.transpose() + D.transpose()*D);

		EigenSolver<MatrixXd> esL(L);
		std::vector<double> lamL = {esL.eigenvalues()[0].real(), esL.eigenvalues()[1].real(), esL.eigenvalues()[2].real()};
		std::sort(lamL.begin(), lamL.end());
		std::reverse(lamL.begin(), lamL.end());

		strain_results_L << data.labels[i] << "\t" << data.points[i].x() << "\t" << data.points[i].y() << "\t" << data.points[i].z() << "\t" << L(0,0) << "\t" << L(1,1) << "\t" << L(2,2) << "\t" << L(0,1) << "\t" << L(1,2) << "\t" << L(1,2) << "\t" << lamL[0] << "\t" << lamL[1] << "\t" << lamL[2] << std::endl;
	}
	strain_results_E.close();
	strain_results_L.close();
/*
	std::cout << endl;
	std::cout << D(0,0) << " " << D(0,1) << " "<< D(0,2) << endl;
	std::cout << D(1,0) << " " << D(1,1) << " "<< D(1,2) << endl;
	std::cout << D(2,0) << " " << D(2,1) << " "<< D(2,2) << endl;
	std::cout << endl;

	std::cout << endl;
	std::cout << E(0,0) << " " << E(0,1) << " "<< E(0,2) << endl;
	std::cout << E(1,0) << " " << E(1,1) << " "<< E(1,2) << endl;
	std::cout << E(2,0) << " " << E(2,1) << " "<< E(2,2) << endl;
	std::cout << endl;

	std::cout << endl;
	std::cout << L(0,0) << " " << L(0,1) << " "<< L(0,2) << endl;
	std::cout << L(1,0) << " " << L(1,1) << " "<< L(1,2) << endl;
	std::cout << L(2,0) << " " << L(2,1) << " "<< L(2,2) << endl;
	std::cout << endl;	
*/
/*
	std::cout << endl << "pos: " << pos[9].x() << "  " << pos[9].y() << "  " << pos[9].z() << "  " << endl;
	std::cout << endl << "status: " << status[9] << endl;
	std::cout << endl << "objmin: " << objmin[9] << endl;
	std::cout << endl << "dis: " << dis[9].x() << "  " << dis[9].y() << "  " << dis[9].z() << "  " << endl;
*/
	// basic vector of points manipulation
	/*
	Point a_point(0.0, 0.0, 0.0);
	double px = 1.1;
	double py = 2.2;
	double pz = 3.3;
	pos.push_back(a_point);
	pos[0].move_to(px,py,pz);
	std::cout << endl << "pos: " << pos[0].x() << "  " << pos[0].y() << "  " << pos[0].z() << "  " << endl;
	*/

/*
	// simple test, strainght line with 5 points
	std::vector<double> x_pos = {};
	x_pos.resize(ndp);

	x_pos[0] = 1.0;
	x_pos[1] = 2.0;
	x_pos[2] = 3.0;
	x_pos[3] = 4.0;
	x_pos[4] = 5.0;

	std::vector<double> y_val = {};
	y_val.resize(ndp);

	y_val[0] = 1.5;
	y_val[1] = 2.8;
	y_val[2] = 3.3;
	y_val[3] = 4.7;
	y_val[4] = 5.1;

	std::cout << endl << "par 1 = " << par(0) << endl << endl;
	std::cout << endl << "par 2 = " << par(1) << endl << endl;
*/

	return 0;
}
/******************************************************************************/
