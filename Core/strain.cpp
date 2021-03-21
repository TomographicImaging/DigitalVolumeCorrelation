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

	// read .disp file into vectors
	std::vector<int> label;
	std::vector<Point> pos;
	std::vector<int> status;
	std::vector<double> objmin;
	std::vector<Point> dis;

	std::string fname;
	fname = "tiny_cloud.disp";
	DispRead disp;
	disp.read_disp_file(fname,label,pos,status,objmin,dis);

	// set up for OLS
	StrainCalc strain;

	int nmp = strain.nmp();		// set by poly model choice, fixed in constructor
	int ndp = 25;				// user set, between a min and max set in constructor
	int npts = pos.size();		// totla number of points in the .disp file

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

	// model used for fitting, 3D quadratic polynomial
	//    m  = c1 
	//       + c2*x + c3*y + c4*z
	//       + c5*x^2 + c6*y^2 + c7*z^2 
	//       + c8*x*y + c9*y*z + c10*x*z


	// for (unsigned int i=0; i<npts; i++) {
	for (unsigned int i=0; i<1; i++) {

		// test point 0 here:

		std::vector<int> nbr_ind = {0,1,5,25,6,26,30,31,2,10,50,7,11,27,35,51,55,32,36,56,12,52,60,3,15,37,57,61,75,8,16,28,40,76,80,33,41,81,62,13,17,53,65,77,85,38,42,58,66,82};

		// load neighborhood vectors npos and ndis for current point and scale

		// scaling

		Point origin(0.0, 0.0, 0.0);
		double distance =  pos[i].pt_dist(origin);
		double scale = 1.0/distance;

		std::vector<Point> npos = {};
		std::vector<Point> ndis = {};

		for (unsigned int j=0; j<ndp; j++) {
			Point spos(scale*pos[nbr_ind[j]].x(), scale*pos[nbr_ind[j]].y(), scale*pos[nbr_ind[j]].z());
			npos.push_back(spos);
			Point sdis(scale*dis[nbr_ind[j]].x(), scale*dis[nbr_ind[j]].y(), scale*dis[nbr_ind[j]].z());
			ndis.push_back(sdis);
		}

		// scale the derivative calculation location
		Point spos(scale*pos[i].x(), scale*pos[i].y(), scale*pos[i].z());


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

	}

	std::cout << endl;
	std::cout << D(0,0) << " " << D(0,1) << " "<< D(0,2) << endl;
	std::cout << D(1,0) << " " << D(1,1) << " "<< D(1,2) << endl;
	std::cout << D(2,0) << " " << D(2,1) << " "<< D(2,2) << endl;
	std::cout << endl;



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
