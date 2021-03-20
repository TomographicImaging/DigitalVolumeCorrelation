#include "strain.h"






/******************************************************************************/
StrainCalc::StrainCalc ()
{

	num_mod_para = 2;	// testing, straight line fit

	num_data_pts = 5;	// testing, straight line fit
	
}
/******************************************************************************/





/******************************************************************************/
int main(int argc, char *argv[])
{

	std::cout << endl << "strain calculation from strain main" << endl << endl;

	StrainCalc strain;

	int nmp = strain.nmp();
	int ndp = strain.ndp();

	std::cout << endl << "nmp local = " << nmp << endl << endl;
	std::cout << endl << "ndp local = " << ndp << endl << endl;

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

	Eigen::VectorXd dv = Eigen::VectorXd(ndp);
	Eigen::MatrixXd Xm = Eigen::MatrixXd(ndp,nmp);
	Eigen::MatrixXd XtXm = Eigen::MatrixXd(nmp,nmp);
	Eigen::VectorXd Xtdv = Eigen::VectorXd(nmp);
	Eigen::VectorXd par = Eigen::VectorXd(nmp);

	for (unsigned int i=0; i<ndp; i++) {
		Xm(i,0) = 1;
		Xm(i,1) = x_pos[i];

		dv(i) = y_val[i];
	}

	XtXm = Xm.transpose()*Xm;
	Xtdv = Xm.transpose()*dv;

	par = XtXm.fullPivHouseholderQr().solve(Xtdv);

	std::cout << endl << "par 1 = " << par(0) << endl << endl;
	std::cout << endl << "par 2 = " << par(1) << endl << endl;

	return 0;
}
/******************************************************************************/
