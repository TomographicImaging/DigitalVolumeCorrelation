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

	// Reserves enough margin (est_box vs act_box) to support B-spline
	// interpolation/gradient evaluation at the given order (3, 5, or 7 --
	// cubic, quintic, septic; throws Intrp_Fail for any other value).
	// Use this constructor whenever tri_bspline / tri_bspline_grad will be
	// called on the resulting object. The other (single-argument) constructor
	// keeps the original frame=1 margin and cannot safely support B-spline
	// evaluation -- calling the B-spline methods on such an object throws.
	Interpolate(const BoundBox *region, int bspline_order);

	~Interpolate();

	void kernels(std::string voxfname, BoundBox *vox_box, int bytes_per, std::string endian, unsigned int offset);
	void kernels(std::string voxfname, BoundBox *vox_box, int bytes_per, std::string endian);

	void kernels_derivs();
	void kernels_Lekien_all();
	void kernels_Lekien_one(int x, int y, int z);

	// Prepares B-spline interpolation coefficients over the *entire* est_box
	// via a separable causal/anticausal recursive (IIR) prefilter, following
	// Unser/Aldroubi/Eden's B-spline signal processing scheme -- the same
	// class of "filter optimization" used in Pan et al., "Accurate B-spline-
	// based 3-D interpolation scheme for digital volume correlation" (Rev.
	// Sci. Instrum. 87, 125114 (2016)). Unlike kernels_Lekien_one/_all this is
	// NOT lazy/per-cell: the recursive filter needs a full line of samples
	// along each axis, so it always processes the whole window. Must be
	// called (again) any time kernels() reloads new data, or after
	// set_bspline_order() changes the active order -- kernels() and
	// set_bspline_order() both mark the coefficients stale, and tri_bspline /
	// tri_bspline_grad throw Intrp_Fail if called while stale, rather than
	// silently interpolating leftover coefficients from a previous window or
	// order. Requires the object to have been constructed with the
	// two-argument constructor above, with an order >= the one currently
	// active; throws Intrp_Fail otherwise.
	void kernels_bspline();

	// Value-only B-spline interpolation. Throws Intrp_Fail if kernels_bspline()
	// hasn't been called since the last kernels() load / set_bspline_order().
	void tri_bspline(const std::vector<Point> &pts, const BoundBox *bbox, std::vector<double> &ivals);

	// B-spline interpolation with analytic first derivatives (index/voxel
	// space, isotropic unit spacing -- consistent with the rest of this
	// class). Caller must size ivals/dfdx/dfdy/dfdz to pts.size(), same
	// convention as nearest/tri_lin/tri_cub_Lek/tri_bspline. Same staleness
	// check as tri_bspline.
	void tri_bspline_grad(const std::vector<Point> &pts, const BoundBox *bbox,
		std::vector<double> &ivals,
		std::vector<double> &dfdx, std::vector<double> &dfdy, std::vector<double> &dfdz);

	// Switches the active B-spline order (3/5/7). Only allowed to select an
	// order that fits within the halo reserved at construction (i.e. you can
	// drop down from septic to cubic on an object built for septic, but not
	// the reverse). Throws Intrp_Fail otherwise. Marks the coefficients stale
	// -- kernels_bspline() must be (re)run before interpolating.
	void set_bspline_order(int order);
	int bspline_order() const { return bsp_order; }

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

	// shared constructor body
	void init(const BoundBox *region, double frame, int bspline_order);

	// --- B-spline support -------------------------------------------------

	int bsp_order;          // active order: 0 (unconfigured), 3, 5, or 7
	int bsp_halo_reserved;  // halo actually reserved in est_box/act_box at construction
	std::vector<double> bsp_poles;  // recursive-filter poles for bsp_order
	double bsp_gain;                // overall prefilter gain for bsp_order
	bool bsp_valid;          // true only between a kernels_bspline() call and the
	                         // next kernels()/set_bspline_order() call that invalidates it

	void set_bspline_poles(int order); // fills bsp_poles/bsp_gain for order (3/5/7)

	// General uniform centered B-spline basis function of the given degree,
	// evaluated at x (any real degree >= 0, not just 3/5/7 -- used directly
	// for the value weights, and via the derivative identity
	// d/dx beta^p(x) = beta^(p-1)(x+0.5) - beta^(p-1)(x-0.5) for gradients).
	static double bspline_basis(int degree, double x);

	// One-dimensional separable prefilter pass (gain + causal/anticausal
	// recursion per pole), applied in place to a single line of samples.
	void bspline_filter_1d(std::vector<double> &c) const;
	static double bspline_init_causal(const std::vector<double> &c, double z, double tolerance);
	static double bspline_init_anticausal(const std::vector<double> &c, double z);
};
/******************************************************************************/

#endif
