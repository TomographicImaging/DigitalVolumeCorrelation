#ifndef INTERPOLATE_H
#define INTERPOLATE_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

// Adjust Makefile if changes made here
#include "Point.h"
#include "BoundBox.h"
#include "Matrix_4d.h"
#include "Utility.h"
//


#include <Eigen/Dense>
#include <Eigen/Sparse>

// using both or either creates 10x speedup with -O3 comp, haven't tracked down the specifics
using namespace Eigen;	
using namespace std;

/******************************************************************************/
class Interpolate
{
public:

	Interpolate(const BoundBox *region);
	~Interpolate();

	void kernels(std::string voxfname, BoundBox *vox_box, int bytes_per, std::string endian);
	void kernels_derivs();
	void kernels_Lekien_all();
	void kernels_Lekien_one(int x, int y, int z);

	void nearest(const std::vector<Point> &pts, const BoundBox *bbox, std::vector<double> &ivals);
	void tri_lin(const std::vector<Point> &pts, const BoundBox *bbox, std::vector<double> &ivals);
	void tri_cub_Lek(const std::vector<Point> &pts, const BoundBox *bbox, std::vector<double> &ivals);

	void center_on(Point pt);

private:
//
// Element [0][0][0] is located at the min corner of the interp_region.
// Subtract est_box.min() from actual (x,y,z) to get relative position.
//
	BoundBox *est_box;	// the overall interp bbox
	BoundBox *act_box;	// the active region, minus a frame for fdd's

	Matrix_4d *kern_4d;

	Eigen::MatrixXd BI;
	Eigen::SparseMatrix<double,Eigen::ColMajor> sBI;
};
/******************************************************************************/

#endif
