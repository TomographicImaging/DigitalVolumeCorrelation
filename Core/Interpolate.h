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
#ifndef INTERPOLATE_H
#define INTERPOLATE_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <omp.h>

// Adjust Makefile if changes made here
#include "Point.h"
#include "BoundBox.h"
#include "Matrix_4d.h"
#include "Utility.h"
//


#include <Eigen/Dense>
#include <Eigen/Sparse>

#include "CCPiDefines.h"

// using both or either creates 10x speedup with -O3 comp, haven't tracked down the specifics
using namespace Eigen;
using namespace std;

/******************************************************************************/
class CCPI_EXPORT Interpolate
{
public:

	Interpolate(const BoundBox *region);
	~Interpolate();

	void kernels(std::string voxfname, BoundBox *vox_box, int bytes_per, std::string endian, unsigned int offset);
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
