/*
Copyright 2018 United Kingdom Research and Innovation
Copyright 2018 Oregon State University

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Author(s): Brian Bay,
           Edoardo Pasca,
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
