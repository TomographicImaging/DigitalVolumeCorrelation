/**
# -*- coding: utf-8 -*-
#   This work is part of the Core Imaging Library developed by
#   Visual Analytics and Imaging System Group of the Science Technology
#   Facilities Council, STFC

#   Copyright 2018 CCPi

#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at

#       http://www.apache.org/licenses/LICENSE-2.0

#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

#   Code is derived from code developed by Prof. Brian Bay
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

	// pointer to the objective function set in RunControl
	double (*obj_fcn)(const std::vector<double> &ref_subvol, const std::vector<double> &tar_subvol);

	int bytes_per;		// keep for now, derived
	double subv_rad;	// keep for now, derived
	int subv_num;		// keep for now, derived

	BoundBox *vox_box;	// image volume dimensions
	BoundBox *est_box_nom;	// for allocating an interpolator kernel

	std::vector<double> ref_subvol;
	std::vector<double> tar_subvol;

	std::vector<double> par_min;	// parameter vector at optimum
	double obj_min;			// objective function value at optimum

	void process_point(int t, int n, DataCloud *srch_data);

	void search_pt_setup(Point srch_pt, std::vector<ResultRecord> &neigh_res);

	void starting_param(Point srch_pt, std::vector<ResultRecord> &neigh_res);

	void look_with(Search_Type method, int ndof, double ftol);

	double obj_val_at(const std::vector<double> x);	// this version uses nominals set at Search construct

	std::vector<double> obj_grad_at(const std::vector<double> x); // simple finite diff, to get started
	std::vector< std::vector<double> > obj_Hess_at(const std::vector<double> x);

	// translation grid style global search
	void trgrid_global(double displ_max, double basin_rad, int n);

	// randomized points style global search
	void random_global(double displ_max, double basin_rad);

	std::vector<double> min_Nelder_Mead(std::vector<double> &start, std::vector<double> &dels, double conv_tol);

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
