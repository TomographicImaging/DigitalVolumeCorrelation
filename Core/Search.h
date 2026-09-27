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
#ifndef SEARCH_H
#define SEARCH_H

#include <stdlib.h>
#include <cstdlib>
#include <stdint.h>
#include <utility>
#include <string>
#include <vector>
#include <time.h>

// Adjust Makefile if changes made here
#include "Point.h"
#include "BoundBox.h"
#include "InputRead.h"
#include "FloatingCloud.h"
#include "DataCloud.h"
#include "Interpolate.h"
#include "ObjectiveFunctions.h"
#include "Utility.h"
//

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include "CCPiDefines.h"

using namespace std;	// keep this here following Eigen includes

/******************************************************************************/
class CCPI_EXPORT Search
{
public:

	Search(RunControl *run);

	~Search();

	RunControl *rc;	// gives all member functions of Search direct access to RunControl without passing
	FloatingCloud *fcld;

	// Interpolators instantiated in dvc_cpp and last for all points in a cloud
	// ref used to load ref_subvol, tar used throughout iteration to load/reload tar_subvolume
	Interpolate *interp_ref;	// persistent interpolator for the reference subvolume
	Interpolate *interp_tar;	// persistent interpolator for the target subvolume

	std::vector<double> ref_subvol;		// vector of subvolume sampling point values ("data" in NLS) extracted from reference subvolume
	std::vector<double> tar_subvol;		// vector of subvolume sampling point values ("model" in NLS) extracted from correlate suvolume

	// pointers to the objective function set in RunControl, versions w/ and w/o return of residual vector
	double (*obj_fcn)(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol);
	double (*obj_fcn_res)(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol, std::vector<double> &residual);

	int bytes_per;		// keep for now, derived
	double subv_rad;	// keep for now, derived
	int subv_num;		// keep for now, derived

	BoundBox *vox_box;	// image volume dimensions

	// these variables are copied into DataCloud on return after process_point
	double obj_min;					// objective function value at optimum for the current cloud point
	std::vector<double> par_min;	// parameter vector at optimum for the current cloud point
	Iter_Stats iter_stats;			// iteration stats for the current cloud point being searched
	
	void process_point(int t, int n, bool map_flag, int map_id, DataCloud *srch_data);

	void search_pt_setup(Point srch_pt, std::vector<ResultRecord> &neigh_res);

	void starting_param(Point srch_pt, std::vector<ResultRecord> &neigh_res);

	double obj_val_at(const std::vector<double> x);	// this version uses nominals set at Search construct
	double obj_val_at(const std::vector<double> x, std::vector<double> &residual); // with residuals returned as well

	void Jacobian_at (const std::vector<double> a, std::vector< std::vector<double> > &J); // by forward diferences: [npts][ndof]
	double LM_prep_at (const std::vector<double> a, VectorXd &e, MatrixXd &J); // key bits needed by LM using eigen library types

	// legacy convergence check
	ConvergenceReason check_convergence(
		const Eigen::VectorXd& X_prev,
		const Eigen::VectorXd& X_curr,
    	double F_prev,
    	double F_curr);

	// enhanced convergence check
	ConvergenceReason Check_Convergence(
		const Eigen::VectorXd& r,
		const Eigen::MatrixXd& J,
		const Eigen::VectorXd& X_prev,
		const Eigen::VectorXd& X_curr,
		double F_prev,
		double F_curr);

	// translation grid style global search
	void trgrid_global(double displ_max, double basin_rad, int n, bool out_as_raw);

	// map objective function for translations surrounding a parameter vector, up to dispalcement max with adjustable increment
	void map_objective_function(int map_id, double half_range, int num_each_dim);

	// randomized points style global search
	void random_global(double displ_max, double basin_rad);

	// the primary optimization method
	ConvergenceReason min_Lev_Mar(const std::vector<double> &start, DataCloud *srch_data);

private:
	// Shared analytic residual-Jacobian assembly for tri_bspline, used by both
	// Jacobian_at() and LM_prep_at() so the two never drift apart. Evaluates
	// at 'a' (length ndof), fills base_res[npts] (the residual vector, same
	// convention as obj_fcn_res) and J[npts][ndof], and returns the objective
	// function value (same convention as obj_val_at). Throws Range_Fail if the
	// query points fall outside the interpolator's active region. See
	// Jacobian_at()'s definition in Search.cpp for the full derivation notes.
	double bspline_jacobian_at(const std::vector<double> &a, int ndof,
		std::vector<double> &base_res, std::vector< std::vector<double> > &J);

#if defined(_WIN32) || defined(__WIN32__)
	//friend CCPI_EXPORT std::ostream& operator<<(std::ostream&, const Search & ) ;
#else
	friend CCPI_EXPORT std::ostream& operator<<(std::ostream&, const Search &);
#endif
};

/******************************************************************************/

#endif
