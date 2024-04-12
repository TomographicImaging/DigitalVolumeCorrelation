/*
Copyright 2018 United Kingdom Research and Innovation
Copyright 2018 Oregon State University

Copyright 2018 Oregon State University
Copyright 2018 United Kingdom Research and Innovation

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

	RunControl *rc;	// give Search direct access to RunControl
	FloatingCloud *fcld;
	Interpolate *interp;

	// pointers to the objective function set in RunControl, versions w/ and w/o return of residual vector
	double (*obj_fcn)(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol);
	double (*obj_fcn_res)(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol, std::vector<double> &residual);

	int bytes_per;		// keep for now, derived
	double subv_rad;	// keep for now, derived
	int subv_num;		// keep for now, derived

	BoundBox *vox_box;	// image volume dimensions
	BoundBox *est_box_nom;	// for allocating an interpolator kernel

	std::vector<double> ref_subvol;
	std::vector<double> tar_subvol;

	std::vector<double> par_min;	// parameter vector at optimum
	double obj_min;			// objective function value at optimum

	void process_point(int t, int n, DataCloud *srch_data, int test = 0);

	void search_pt_setup(Point srch_pt, std::vector<ResultRecord> &neigh_res);

	void starting_param(Point srch_pt, std::vector<ResultRecord> &neigh_res);

	void look_with(Search_Type method, int ndof, double ftol);

	double obj_val_at(const std::vector<double> x);	// this version uses nominals set at Search construct
	double obj_val_at(const std::vector<double> x, std::vector<double> &residual); // with residuals returned as well

	void Jacobian_at (const std::vector<double> a, std::vector< std::vector<double> > &J); // by forward diferences: [npts][ndof]
	double LM_prep_at (const std::vector<double> a, VectorXd &e, MatrixXd &J); // key bits needed by LM using eigen library types

	std::vector<double> obj_grad_at(const std::vector<double> x); // simple finite diff if E, overall obj value
	std::vector< std::vector<double> > obj_Hess_at(const std::vector<double> x);

	// translation grid style global search
	void trgrid_global(double displ_max, double basin_rad, int n);

	// randomized points style global search
	void random_global(double displ_max, double basin_rad);

	std::vector<double> min_Nelder_Mead(std::vector<double> &start, std::vector<double> &dels, double conv_tol);
	std::vector<double> min_Lev_Mar(const std::vector<double> &start, const double obj_tol, const double mag_tol);

//	struct Min_Ftor_new;	// leave declared for now, not working
private:
#if defined(_WIN32) || defined(__WIN32__)
	//friend CCPI_EXPORT std::ostream& operator<<(std::ostream&, const Search & ) ;
#else
	friend CCPI_EXPORT std::ostream& operator<<(std::ostream&, const Search &);
#endif
};

/******************************************************************************/
// not currently used, but leave in place for now
/*struct Search::Min_Ftor_new
{
	int lndof;
	FloatingCloud *lfcld;
	Interpolate *linterp;

	std::vector<double> lpar_cur;
	std::vector<double> lref_subvol;
	std::vector<double> ltar_subvol;

	Objfcn_Type lobj_typ;
	Interp_Type lint_typ;


	Min_Ftor_new(const Interp_Type int_typ, const Objfcn_Type obj_typ, const int ndof, FloatingCloud *fcld, Interpolate *interp, const std::vector<double> &ref_subvol);

	double operator() (const std::vector<double> x);

};*/
/******************************************************************************/

#endif
