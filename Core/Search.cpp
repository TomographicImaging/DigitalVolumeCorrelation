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
#include "Search.h"

/******************************************************************************/
Search::Search(RunControl *run)
{
	rc = run;	// gives all member functions of Search direct access to RunControl without passing

	bytes_per = rc->vol_bit_depth/8;
	subv_rad = (rc->subvol_size)/2.0;	// easier to use than subvol_size
	if (rc->sub_geo == Subvol_Type::cube) subv_num = pow(ceil(pow((double)rc->subvol_npts,1.0/3.0)),3.0);
	if (rc->sub_geo == Subvol_Type::sphere) subv_num = rc->subvol_npts;

	// establish pointer to function set in input file
	switch (run->obj_fcn) {
		case SSD:
			obj_fcn = &obj_SSD;
			obj_fcn_res = &obj_SSD;
			break;
		case ZSSD:
			obj_fcn = &obj_ZSSD;
			obj_fcn_res = &obj_ZSSD;
			break;
		case NSSD:
			obj_fcn = &obj_NSSD;
			obj_fcn_res = &obj_NSSD;
			break;
		case ZNSSD:
			obj_fcn = &obj_ZNSSD;
			obj_fcn_res = &obj_ZNSSD;
			break;
	}

	// create reference and correlate (target) subvolume vectors of interpolated voxel data, init to 0.0
	ref_subvol = std::vector<double>(subv_num,0.0);
	tar_subvol = std::vector<double>(subv_num,0.0);

	// optimization result storage 
	par_min = std::vector<double>(rc->num_srch_dof,0.0);

	// Notes on boxes:
	//	1. They do not contain content, they simply define the boundaries of rectangular prisms in the 3D voxel space. 
	//	2. Used within the code to define processing regions, validate input parameters, and control processing.
	//	3. Boxes are nested to support interpolator construction and manage the movement of subvolume sampling points during optimization. 
	//	4. Boxes are inherently float, but some uses of boxes (i.e. interpolators) are inherently int and need padding - see grow_by(1.0) below.
	//
	// *** box descriptions (small to large):
	//	1. act_box = (active) accessible region for interpolation returns within an est_box, inset by the frame (halo) required for coefficient calculation
	//	2. est_box = (estimation) total voxel prism of an interpolator, including the the frame region reguired for coefficient calculation
	//	3. vox_box = (voxel) limits of the full image volume space, e.g. a raw file of 1500x2000x2500 has corners at (0,0,0) and (1500,2000,2500)

	// create a box with the full dimensions of the image voxel volumes for use in range checking
	Point vox_box_min(0.0, 0.0, 0.0);
	Point vox_box_max(rc->vol_wide, rc->vol_high, rc->vol_tall);
	vox_box = new BoundBox(vox_box_min, vox_box_max);

	// form boxes of suitable size for both ref and tar Interpolators
	// Instantiate depending on bspline or linear/cubic settings
	// the interpolators are instantiated ahead of point loop in dvc.cpp and persist for all points in a cloud
	// (note ... if modifing for subregion instead of single point process this will need reconsideration, interp_tar will need to size adjust)

	// Interpolate constructor calls an init function that manages the frame (halo) region needed for the boxes
	// no kernels are set here, that requires position information for the reference cloud point and search start positions
	// see process_point -> search_pt_setup for kernel calulation calls for ref and tar and one-time interpolation taps for ref
	// see min_Lev_Mar -> LM_prep_at -> bspline_jacobian_at/obj_val_at for interpolation taps needed to load tar_subvol at each iteration

	// there is a subtle aspect to establishing the box sizes
	// Interpolate construction is inherently voxel-based, with kernel information associated with integer voxel cneters
	// however, subvolume sampling mini-clouds are floating non-integer values
	// integer portion of the point locations is used for setting up space for kernel storage
	// remainder portion is used for positioning interpolation taps within kernel cells
	// this leaves the bounding subvolume sampling points in an overflow position, requiring the grow_by(1.0) just before instantiation

	// create a box the size of the active region for interp_ref (just the subvolume size)
	Point act_box_nom_min = Point(0.0, 0.0, 0.0);
	Point act_box_nom_max = Point(2*subv_rad*rc->subvol_aspect[0], 2*subv_rad*rc->subvol_aspect[1], 2*subv_rad*rc->subvol_aspect[2]);
	BoundBox act_box_ref_nom = BoundBox(act_box_nom_min, act_box_nom_max);	// box without the additional space for search, no grow_by

	// create another box of subvolume size, then enlarge to accomodate the search region
	BoundBox act_box_tar_nom = BoundBox(act_box_nom_min, act_box_nom_max);
	act_box_tar_nom.grow_by(rc->step_max);		// this increases act_box by step_max to accomdate subvolume movement during iteration

	if (rc->bspline == true) {
		const int bspline_order_cfg = rc->bspline_order;
		act_box_tar_nom.grow_by(1.0);	// account for int truncation of point cloud dimensions used for voxel ranges
		act_box_ref_nom.grow_by(1.0);
		interp_ref = new Interpolate(&act_box_ref_nom, bspline_order_cfg);
		interp_tar = new Interpolate(&act_box_tar_nom, bspline_order_cfg);	
	}
	else {
		// compensate for derivatives in interpolator (fixed frame=1.0 internally, so this nets +1.0 margin)
		act_box_tar_nom.grow_by(1.0);
		act_box_ref_nom.grow_by(1.0);
		// this is the tricubic constructor
		interp_ref = new Interpolate(&act_box_ref_nom);
		interp_tar = new Interpolate(&act_box_tar_nom);
	}

	// diagnostic
	/*
	std::cout << std::endl << "interp (persistent, Search constructor) ..." << std::endl;
	int wide, high, tall;
	interp_tar->act_box_dims(wide, high, tall);
	std::cout << "interp_tar act_box = " << wide << " " << high << " " << tall << std::endl;
	interp_tar->est_box_dims(wide, high, tall);
	std::cout << "interp_tar est_box = " << wide << " " << high << " " << tall << std::endl;

	interp_ref->act_box_dims(wide, high, tall);
	std::cout << "interp_ref act_box = " << wide << " " << high << " " << tall << std::endl;
	interp_ref->est_box_dims(wide, high, tall);
	std::cout << "interp_ref est_box = " << wide << " " << high << " " << tall << std::endl;

	std::cout << "... done" << std::endl;
	*/
	//

}
/******************************************************************************/
Search::~Search()
{
	// dispose of persistent class instantiations 
	delete interp_tar;
	delete interp_ref;
	delete vox_box;
	//delete est_box_nom;
	delete fcld;
}
/******************************************************************************/
void Search::process_point(int t, int n, bool map_flag, int map_id, DataCloud *srch_data)
{
	// n is the index of the current search point
	Point srch_pt = srch_data->points[n];

	// subvolume sampling mini-clouds are created here for each clouf point to support randomized sampling positions
	if (rc->sub_geo == sphere) {
		fcld = new FloatingCloud(srch_pt, subv_rad, subv_num, rc->subvol_aspect[0], rc->subvol_aspect[1], rc->subvol_aspect[2]);
	}

	if (rc->sub_geo == Subvol_Type::cube) {
		Point sub_min = srch_pt;
		sub_min.move_by(-subv_rad*rc->subvol_aspect[0], -subv_rad*rc->subvol_aspect[1], -subv_rad*rc->subvol_aspect[2]);
		Point sub_max = srch_pt;
		sub_max.move_by(subv_rad*rc->subvol_aspect[0], subv_rad*rc->subvol_aspect[1], subv_rad*rc->subvol_aspect[2]);
		int grid_inc = ceil(pow((double)subv_num,1.0/3.0));
		fcld = new FloatingCloud(sub_min, sub_max, grid_inc, grid_inc, grid_inc);
	}

	// vector of ResultRecord for the neighborhood of this search point
	std::vector<ResultRecord> neigh_res;
	for (int i=0; i<srch_data->neigh[n].size(); i++) {
		neigh_res.push_back(srch_data->results[t][srch_data->neigh[n][i]]);
	}

	// this sets par_min to the starting point estimate through starting_param call
	// fills ref_subvolume vector for the search point byt interpolating within the ref volume
	// prepares kernels for subsequent interpolation filling of tar_subvolume during iteration

	search_pt_setup(srch_pt, neigh_res);

	// *** special cases

	// trap point for objective function mapping
	if (map_flag && (srch_data->labels[n] == map_id)) {
		std::cout << std::endl << "** mapping point with label " << map_id << std::endl;
		// mapping range is taken from the step_max value in the dvc input file
		double half_range = rc->step_max;
		int num_each_dim = 100;		// hardcode this for now, could be a command line input in future
		map_objective_function(map_id, half_range, num_each_dim);
	}

	// coarse search step, this is probably going to phase out, except perhaps for optimization start refinement in special cases
	// removed from input file, code blocks remain but not called
	// basin_radius = 0.0 in the input file signals no coarse search step
	// if (rc->basin_radius > 0.0) {trgrid_global(rc->step_max, rc->basin_radius, n, false);}
	// random search is also reserved for optimization start refinement in special cases, not triggered in current code configuration
	//	random_global(rc->step_max, rc->basin_radius);

/*******************/

	// L-M optimization, currently using pure QN steps (no lambda tuning)

	// par_min initial estimate in place here, with par_min reset within min_Lev_Mar, copy into start (passed as const)
	int ndof = par_min.size();
	std::vector<double> start(ndof, 0.0);
	for (int i=0; i<ndof; i++) {
		start[i] = par_min[i];
	}

	// par_min and obj_min set within min_Lev_Mar, returns a ConvergenceReason that includes Converged and Maxit codes

	ConvergenceReason opt_status = min_Lev_Mar(start, srch_data);

	//std::cout << "convg_result = " << CR_str_vec[opt_status] << std::endl;

	if (opt_status == Converged || opt_status == CostChange || opt_status == ParameterChange || opt_status == GradientNorm) {
		throw Point_Good();
	}
	if (opt_status == Maxit || opt_status == NotConverged) {
		throw Convg_Fail();
	}

	// Range_Fail is coming through now for very small search region settings. 
}
/******************************************************************************/
void Search::search_pt_setup(Point srch_pt, std::vector<ResultRecord> &neigh_res)
{
	// target volume interpolation preparation
	// reading of image data and kernel calculations for the (subvolume) reference box
	// interpoaltion taps are done here for each search point to fill the ref_subvol vector

	interp_ref->center_on(srch_pt);
	interp_ref->kernels(rc->ref_fname, vox_box, bytes_per, rc->vol_endian, rc->vol_hdr_lngth);

	if (rc->int_typ == trilinear) {
		interp_ref->tri_lin(fcld->stable->ptvect, fcld->stable->bbox(), ref_subvol);
	}
	if (rc->int_typ == tricubic) {
		interp_ref->tri_cub_Lek(fcld->stable->ptvect, fcld->stable->bbox(), ref_subvol);
	}
	if (rc->bspline == true) {
		interp_ref->kernels_bspline();
		interp_ref->tri_bspline(fcld->stable->ptvect, fcld->stable->bbox(), ref_subvol);
	}

	// target volume interpolation preparation
	// reading of image data and kernel calculations for the (subvolume + step_max) target box
	// no interpolation taps at this stage
	// requires adjustment of subvolume sampling locations for the current optimization parameter vector values

	starting_param(srch_pt, neigh_res);

	Point offset_pt = srch_pt;
	offset_pt.move_by(par_min[0], par_min[1], par_min[2]);
	interp_tar->center_on(offset_pt);

	// this covers tri_lin/tri_cub_leK and partially grenerates tri_bspline
	interp_tar->kernels(rc->cor_fname, vox_box, bytes_per, rc->vol_endian, (unsigned int)rc->vol_hdr_lngth);

	// this completes kernel development when tri_bspline options are active
	if (rc->bspline == true) {
		interp_tar->kernels_bspline();
	}
}
/******************************************************************************/
// legacy version, simple checks on relative objective function and parameter vector changes
ConvergenceReason Search::check_convergence(
    const Eigen::VectorXd& X_prev,
    const Eigen::VectorXd& X_curr,
    double F_prev,
    double F_curr)	// access is tol.cost_tol, tol.step_tol, tol.grad_tol,
{
	// Notes on convergence.
	// 1. These simple checks behave in a very similar way to C_C with similar settings
	// 2. Gets to CostChange even at 1e-10 with just a couple more iterations. 
	// 3. But this looks artificial for settings of this scale, as compared with C_C which does not converge. 

	double tol_cost = 1e-6;			// hardcoded locally here for now, (legacy = 1e-6, reset to 1e-6)
	double tol_step = 1e-4;			// hardcoded locally here for now, (legacy = 1e-2, reset to 1e-4)

	// 1. Change in the objective function:
	double cost_norm = fabs(F_curr - F_prev);
	if (cost_norm < tol_cost) {
		return ConvergenceReason::CostChange;
	}

	// 2. Change in the parameter vector:
	double step_norm = (X_curr - X_prev).norm();
	if (step_norm < tol_step) {
		return ConvergenceReason::ParameterChange;
	}

	return NotConverged;	// if none of the checks are satisfied this is returned
}
/******************************************************************************/
ConvergenceReason Search::Check_Convergence(
    const Eigen::VectorXd& r,
    const Eigen::MatrixXd& J,
    const Eigen::VectorXd& X_prev,
    const Eigen::VectorXd& X_curr,
    double F_prev,
    double F_curr)
{
	// tolerances set in header

    // 1. Change in the objective function:
    //    |S(X_prev) - S(x_{k+1})| / S(X_prev) < eps1
    double cost_denom = std::max(F_prev, std::numeric_limits<double>::epsilon());
    double cost_change = std::abs(F_prev - F_curr) / cost_denom;
    if (cost_change < rc->cost_tol) {
        return ConvergenceReason::CostChange;
    }

//	std::cout << std::setprecision(3) << std::scientific;
//	std::cout << "rc->cost_tol " << rc->cost_tol << std::endl;

    // 2. Change in the parameter vector:
    //    ||x_{k+1} - X_prev|| < eps2 * (||X_prev|| + eps2)
    double step_norm  = (X_curr - X_prev).norm();
    double param_norm = X_prev.norm();
    if (step_norm < rc->step_tol * (param_norm + rc->step_tol)) {
        return ConvergenceReason::ParameterChange;
    }

    // 3. Gradient norm (first-order optimality):
    //    ||J^T r||_inf < eps3
 //   Eigen::VectorXd grad = J.transpose() * r;   // = gradient of S(x) = 0.5||r||^2
 //   double grad_inf_norm = grad.lpNorm<Eigen::Infinity>();
 //   if (grad_inf_norm < grad_tol) {
 //       return ConvergenceReason::GradientNorm;
 //   }

    return ConvergenceReason::NotConverged;
}
/******************************************************************************/
ConvergenceReason Search::min_Lev_Mar(const std::vector<double> &start, DataCloud *srch_data)
// obj_tol compares with change in the objective function at each iteration
// pos_tol compares with position change at each iteration
{
	int npts = subv_num;
	int ndof = start.size();

	ConvergenceReason convg_status;
	Iter_Stats point_stats;

	std::vector<double> X_prev(ndof, 0.0);		// parameter vector of previous  iteration
	std::vector<double> X_curr(ndof, 0.0);		// parameter vector of current iteration

	double F_prev{};		// objective function value of previous iteration, itialized to zero
	double F_curr{};		// objective function value of current iteration, itialized to zero

	// patch for now to support Check_Convergence which uses Eigen .norm, lpNorm, etc.
	// switch to all Eigen types in future
	Eigen::VectorXd X_prev_eig = Eigen::VectorXd(ndof);		// parameter vector of previous  iteration
	Eigen::VectorXd X_curr_eig = Eigen::VectorXd(ndof);		// parameter vector of current iteration

	// update variables
	Eigen::VectorXd r = Eigen::VectorXd(npts);			// residual vector
	Eigen::MatrixXd J = Eigen::MatrixXd(npts,ndof);		// Jacobian
	Eigen::MatrixXd JTJ = Eigen::MatrixXd(ndof,ndof);	// lhs
	Eigen::VectorXd JTr = Eigen::VectorXd(ndof);		// rhs
	Eigen::VectorXd update = Eigen::VectorXd(ndof);		// parameter change

	// X contains the updated parameter vector as optimization proceeds, initialized here
	for (int i=0; i<ndof; i++) {
			X_curr[i] = start[i];
	}

	// track number of iterations, start with 1 for nits updated within the convergence check conditional
	int nits = 1;

	// update iteration loop
	for (int i=0; i<rc->max_iter; i++) {

		F_curr = LM_prep_at(X_curr, r, J);	// X is a constant input, r and J passed as pointers and modified

		if (i==0) { iter_stats.obj_beg = F_curr; }

		// convergence check
		if (i>0) {
			nits += 1;

			for (int j=0; j<ndof; j++) {
				X_prev_eig(j) = X_prev[j];
				X_curr_eig(j) = X_curr[j];
			}

			convg_status = Check_Convergence(r, J, X_prev_eig, X_curr_eig, F_prev, F_curr);

			if (convg_status != NotConverged) {
				for (int j=0; j<ndof; j++) {
					par_min[j] = X_curr[j];
				}
				obj_min = F_curr;
				break;
			}
		}

		// *** No Convergence at maxit
		if (i == rc->max_iter - 1){
			for (int j=0; j<ndof; j++) {
					par_min[j] = 0.0;
			}
			obj_min = 0.0;
			convg_status = ConvergenceReason::Maxit;
			break;
		}

		// determine and apply a parameter update, move current data back to previous
		JTJ = J.transpose()*J;
		JTr = J.transpose()*r;
		update = JTJ.colPivHouseholderQr().solve(-JTr);
		
		for (int j=0; j<ndof; j++) {
			X_prev[j] = X_curr[j];
			X_curr[j] += update(j);
		}
		F_prev = F_curr;
	}

	iter_stats.nits = nits;
	iter_stats.convg_status = convg_status; 

	return convg_status;
}
/******************************************************************************/
void Search::starting_param(Point srch_pt, std::vector<ResultRecord> &neigh_res)
{
/*
	// *** this gets first (closest) successful neighbor
	double past_srch_x = 0.0;
	double past_srch_y = 0.0;
	double past_srch_z = 0.0;
	int good = 0;
	for (int i=0; i<neigh_res.size(); i++) {
		if (neigh_res[i].status == point_good) {
			past_srch_x = neigh_res[i].par_min[0];
			past_srch_y = neigh_res[i].par_min[1];
			past_srch_z = neigh_res[i].par_min[2];
			good += 1;
			break;
		}
	}
//	std::cout << "\n" << "good = " << good << "\t" << "est = " << past_srch_x << "\t" << past_srch_y << "\t" << past_srch_z << "\n";
*/

	// *** this gets average of all successful neighbors
	double past_srch_x = 0.0;
	double past_srch_y = 0.0;
	double past_srch_z = 0.0;
//	double sum_inv_objmin = 0.0;		// for weighted averaging of nssd and znssd
	// inverse of obj_min is aggressive, will break for autocorrelation
	// might not be beneficial, little effect in initial tests
	int good = 0;
	for (int i=0; i<neigh_res.size(); i++) {
		if (neigh_res[i].status == point_good) {
			past_srch_x += neigh_res[i].par_min[0];
			past_srch_y += neigh_res[i].par_min[1];
			past_srch_z += neigh_res[i].par_min[2];
//			if (rc->obj_fcn==NSSD || rc->obj_fcn==ZNSSD ) {
//				sum_inv_objmin += 1.0/neigh_res[i].obj_min;
//			}
			good += 1;
		}
	}
	if (good != 0) {
		// simple average of all successful neighbors
		past_srch_x /= good;
		past_srch_y /= good;
		past_srch_z /= good;

		// weighted average of all successful neighbors
		/*
		if (rc->obj_fcn==NSSD || rc->obj_fcn==ZNSSD ) {
			past_srch_x = 0.0;
			past_srch_y = 0.0;
			past_srch_z = 0.0;
			for (int i=0; i<neigh_res.size(); i++) {
				if (neigh_res[i].status == point_good) {
					double weight = (1.0/neigh_res[i].obj_min)/sum_inv_objmin;
					past_srch_x += weight * neigh_res[i].par_min[0];
					past_srch_y += weight * neigh_res[i].par_min[1];
					past_srch_z += weight * neigh_res[i].par_min[2];
				}
			}
		}
		*/
		//

	}
	// std::cout << "good = " << good << "\t" << "est = " << past_srch_x << "\t" << past_srch_y << "\t" << past_srch_z << "\n";

	// *** use starting information to form initial parameter vector
	// note: with no past points found and no start_estimate set initial becomes zero

	if (good != 0)
	{
		par_min[0] =  past_srch_x;
		par_min[1] =  past_srch_y;
		par_min[2] =  past_srch_z;
	} else
	{
		par_min[0] =  rc->start_estimate[0];
		par_min[1] =  rc->start_estimate[1];
		par_min[2] =  rc->start_estimate[2];
	}

	// *** set higher-order initial values to zero for now
	// need to add at least rotation in future
	if (rc->num_srch_dof > 3)
	{
		par_min[3] =  0.0;
		par_min[4] =  0.0;
		par_min[5] =  0.0;
	}

	if (rc->num_srch_dof > 6)
	{
		par_min[6] =  0.0;
		par_min[7] =  0.0;
		par_min[8] =  0.0;
		par_min[9] =  0.0;
		par_min[10] =  0.0;
		par_min[11] =  0.0;
	}

}
/******************************************************************************/
double Search::obj_val_at(const std::vector<double> x)	// this version uses nominals set at Search construct
{
	fcld->affine_to(x, x.size());

	if (rc->int_typ == trilinear) {
		try {interp_tar->tri_lin(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}}

	if (rc->int_typ == tricubic) {
		try {interp_tar->tri_cub_Lek(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}}

	if (rc->bspline == true) {
		// kernels_bspline() is NOT called here -- it's primed once in
		// search_pt_setup() right after the moving-cloud kernels() load, and
		// stays valid (bsp_valid) across every obj_val_at() call in this
		// search point's optimization loop, since fcld->affine_to() only
		// moves query points, it never reloads voxel data.
		try {interp_tar->tri_bspline(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}}

	double obj_val = obj_fcn(ref_subvol, tar_subvol);

	return obj_val;
}
/******************************************************************************/
double Search::obj_val_at(const std::vector<double> x, std::vector<double> &residual)	// this version returns residual vector as well
{
	fcld->affine_to(x, x.size());

	if (rc->int_typ == trilinear) {
		try {interp_tar->tri_lin(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}}

	if (rc->int_typ == tricubic) {
		try {interp_tar->tri_cub_Lek(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}}

	if (rc->bspline == true) {
		// see obj_val_at(x) above -- kernels_bspline() is primed once in
		// search_pt_setup(), not on every call here.
		try {interp_tar->tri_bspline(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}}

	double obj_val = obj_fcn_res(ref_subvol, tar_subvol, residual);

	return obj_val;
}
/******************************************************************************/
double Search::LM_prep_at (const std::vector<double> a, VectorXd &e, MatrixXd &J)
{
	int npts = e.size();
	int ndof = a.size();

	std::vector<double> base_res(npts, 0.0);

	if (rc->bspline == true)
	{
		// same analytic Jacobian as Jacobian_at() -- see bspline_jacobian_at()
		// for the derivation. This is the path min_Lev_Mar() actually drives,
		// so this is where the analytic gradient pays off during a real search.
		std::vector< std::vector<double> > Jm(npts, std::vector<double>(ndof, 0.0));

		double obj = bspline_jacobian_at(a, ndof, base_res, Jm);

		for (int j=0; j<npts; j++)
		{
			e(j) = base_res[j];
			for (int i=0; i<ndof; i++)
				J(j,i) = Jm[j][i];
		}

		return obj;
	}

	// --- legacy finite-difference path (trilinear/Lekien tricubic) ---

	std::vector<double> step_res(npts, 0.0);
	std::vector<double> step_x(ndof, 0.0);

	double obj = obj_val_at(a, base_res);
	// double h = 1E-8;
	double h = 1E-10;

	for (int i=0; i<ndof; i++)
	{
		step_x = a;
		step_x[i] += h;
		obj_val_at(step_x, step_res);
		for (int j=0; j<npts; j++)
		{
			J(j,i) = (step_res[j] - base_res[j])/h;
		}
	}

	for (int j=0; j<npts; j++)
	{
		e(j) = base_res[j];
	}

	//std::cout << "\n" << std::setprecision(20);
	//std::cout << J(0,0) << "\t" << J(npts-1,ndof-1) << "\t" << "\n";
	//std::cout << e(0) << "\t" << e(npts-1) << "\n";
	//std::cout << "\n" << std::setprecision(6);

	return obj;
}
/******************************************************************************/
double Search::bspline_jacobian_at(const std::vector<double> &a, int ndof,
	std::vector<double> &base_res, std::vector< std::vector<double> > &J)
// Analytic residual Jacobian using tri_bspline_grad -- ONE interpolation call
// total (vs. ndof+1 full volume interpolations in the legacy finite-difference
// path used by trilinear/Lekien tricubic). Shared by Jacobian_at() and
// LM_prep_at() so the two never drift apart.
//
// This is the standard DIC/DVC "steepest descent image" construction:
// chain-rule the analytic intensity gradient (dfdx/dfdy/dfdz, from
// tri_bspline_grad, evaluated once at the current parameter vector 'a')
// through the shape-function (affine transform) Jacobian d(point)/d(parameter).
// The transform's own parameter->matrix mapping (SearchParams::matr_rot/
// tens_str) is NOT needed explicitly: d(point)/d(parameter) is obtained
// instead by calling FloatingCloud::affine_to() itself on perturbed parameter
// vectors and reading back the resulting point positions -- pure
// vector/matrix arithmetic over the subvolume's points, no volume
// interpolation, so this is negligible cost regardless of ndof. Translation
// dof's (indices 0-2) are exact, not finite-differenced: affine_to() moves
// every point by exactly (del_x,del_y,del_z) independent of any
// rotation/strain, so d(point)/d(translation) is exactly the identity.
//
// The per-point-per-dof geometric*intensity term (G below) is then run
// through the same normalization each obj_fcn_res variant applies
// (mean-subtraction for Z*, sum-of-squares scaling for N*), so the result
// matches obj_fcn_res's residual formula exactly -- see
// ObjectiveFunctions.cpp for the residual definitions this mirrors.
{
	int npts = (int)base_res.size();

	std::vector<double> dfdx(npts), dfdy(npts), dfdz(npts);

	fcld->affine_to(a, ndof);
	try {interp_tar->tri_bspline_grad(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol, dfdx, dfdy, dfdz);}
	catch (Intrp_Fail) {throw Range_Fail();}

	double obj = obj_fcn_res(ref_subvol, tar_subvol, base_res);	// fills base_res

	std::vector< std::vector<double> > G(npts, std::vector<double>(ndof, 0.0));

	const double h = 1e-6;	// pure-geometry central diff -- no interpolation noise, generous h is fine
	std::vector<double> step_x(ndof);

	for (int i=0; i<ndof; i++)
	{
		if (i < 3)
		{
			for (int j=0; j<npts; j++)
				G[j][i] = (i==0) ? dfdx[j] : (i==1) ? dfdy[j] : dfdz[j];
			continue;
		}

		step_x = a;
		step_x[i] += h;
		fcld->affine_to(step_x, ndof);
		std::vector<Point> plus_pts = fcld->moving->ptvect;

		step_x[i] -= 2.0*h;
		fcld->affine_to(step_x, ndof);
		std::vector<Point> minus_pts = fcld->moving->ptvect;

		for (int j=0; j<npts; j++)
		{
			double dxda = (plus_pts[j].x() - minus_pts[j].x()) / (2.0*h);
			double dyda = (plus_pts[j].y() - minus_pts[j].y()) / (2.0*h);
			double dzda = (plus_pts[j].z() - minus_pts[j].z()) / (2.0*h);
			G[j][i] = dfdx[j]*dxda + dfdy[j]*dyda + dfdz[j]*dzda;
		}
	}

	// restore fcld->moving to reflect 'a' (we perturbed away from it above)
	fcld->affine_to(a, ndof);

	if (rc->obj_fcn == SSD)
	{
		// residual_j = tar_j - ref_j  =>  d(residual_j)/d(a_i) = G[j][i]
		for (int j=0; j<npts; j++)
			for (int i=0; i<ndof; i++)
				J[j][i] = G[j][i];
	}
	else if (rc->obj_fcn == ZSSD)
	{
		// residual_j = (tar_j-avg_tar) - (ref_j-avg_ref)
		// => d(residual_j)/d(a_i) = G[j][i] - avg_i(G)
		for (int i=0; i<ndof; i++)
		{
			double avg_g = 0.0;
			for (int j=0; j<npts; j++) avg_g += G[j][i];
			avg_g /= npts;

			for (int j=0; j<npts; j++)
				J[j][i] = G[j][i] - avg_g;
		}
	}
	else if (rc->obj_fcn == NSSD)
	{
		// residual_j = tar_j/Ct - ref_j/Cr,  Ct = sqrt(sum_k tar_k^2)
		// => d(residual_j)/d(a_i) = G[j][i]/Ct - tar_j*S_i/Ct^3,  S_i = sum_k tar_k*G[k][i]
		double Ct2 = 0.0;
		for (int j=0; j<npts; j++) Ct2 += tar_subvol[j]*tar_subvol[j];
		double Ct = sqrt(Ct2);

		for (int i=0; i<ndof; i++)
		{
			double S_i = 0.0;
			for (int k=0; k<npts; k++) S_i += tar_subvol[k]*G[k][i];

			for (int j=0; j<npts; j++)
				J[j][i] = G[j][i]/Ct - tar_subvol[j]*S_i/(Ct*Ct*Ct);
		}
	}
	else if (rc->obj_fcn == ZNSSD)
	{
		// residual_j = (tar_j-avg_tar)/Ctb - (ref_j-avg_ref)/Crb,  u_j = tar_j-avg_tar,
		// Ctb = sqrt(sum_k u_k^2)
		// => d(residual_j)/d(a_i) = (G[j][i]-avg_i(G))/Ctb - u_j*T_i/Ctb^3,  T_i = sum_k u_k*G[k][i]
		double avg_tar = 0.0;
		for (int j=0; j<npts; j++) avg_tar += tar_subvol[j];
		avg_tar /= npts;

		std::vector<double> u(npts);
		double Ctb2 = 0.0;
		for (int j=0; j<npts; j++)
		{
			u[j] = tar_subvol[j] - avg_tar;
			Ctb2 += u[j]*u[j];
		}
		double Ctb = sqrt(Ctb2);

		for (int i=0; i<ndof; i++)
		{
			double avg_g = 0.0;
			for (int j=0; j<npts; j++) avg_g += G[j][i];
			avg_g /= npts;

			double T_i = 0.0;
			for (int k=0; k<npts; k++) T_i += u[k]*G[k][i];

			for (int j=0; j<npts; j++)
				J[j][i] = (G[j][i]-avg_g)/Ctb - u[j]*T_i/(Ctb*Ctb*Ctb);
		}
	}

	return obj;
}
/******************************************************************************/
void Search::Jacobian_at (const std::vector<double> a, std::vector< std::vector<double> > &J)
// a[ndof]
// J[npts][ndof]
{
	int npts = J.size();
	int ndof = J[0].size();

	std::vector<double> base_res(npts, 0.0);

	if (rc->bspline == true)
	{
		bspline_jacobian_at(a, ndof, base_res, J);
		return;
	}

	// --- legacy finite-difference path (trilinear/Lekien tricubic) ---

	std::vector<double> step_res(npts, 0.0);
	std::vector<double> step_x(ndof, 0.0);

	obj_val_at(a, base_res);
	double h = 1E-8;

	for (int i=0; i<ndof; i++)
	{
		step_x = a;
		step_x[i] += h;
		obj_val_at(step_x, step_res);
		for (int j=0; j<npts; j++)
		{
			J[j][i] = (step_res[j] - base_res[j])/h;
		}
	}
std::cout << "\n" << std::setprecision(20);
std::cout << J[0][0] << "\t" << J[npts-1][ndof-1] << "\t" << "\n";
std::cout << "\n" << std::setprecision(6);

}
/******************************************************************************/
void Search::trgrid_global(double displ_max, double basin_radius, int n, bool out_as_raw)
{

	std::cout << "trigrid_global running" << std::endl;

	if (out_as_raw) {
		std::cout << std::endl << "** in trigrid_global with out_as_raw true and point number in cloud  " << n << std::endl;
		// return;
	}

	// use basin_fraction as a basis for controlloing the resolution of the global search
	// 0.0 < basin_fraction <= 1.0, check on input
	// full float version, very slow with tricubic, much faster with trilinear

	// check for opt out of coarse search
	if ((out_as_raw == false) && basin_radius == 0.0) return;

	// temp!
	if (out_as_raw) basin_radius = 2.0;

	int num_inc = displ_max/basin_radius;
	double inc = displ_max/num_inc;
	int num_pts = (2*num_inc+1)*(2*num_inc+1)*(2*num_inc+1);

	// objective function mapping
	// change to appended array instead of full size allocate
	std::vector<double> obj_vals(num_pts,0.0);
	//
	int ndof = 3;	// just search global with translation dof's

	std::vector<double> par_cur(ndof,0.0);

	double obj_min = std::numeric_limits<double>::max();
	double obj_max = std::numeric_limits<double>::min();	// for scaling output

	double obj_val = 0.0;
	int min_x = 0;
	int min_y = 0;
	int min_z = 0;
	int count = 0;

	for (int z=-num_inc; z<=num_inc; z++)
	for (int y=-num_inc; y<=num_inc; y++)
	for (int x=-num_inc; x<=num_inc; x++) {

		std::cout << "count/num_pts = " << count << "/" << num_pts << std::endl;


// for output subvol checks
//	for (int z=0; z<=0; z++)
//	for (int y=0; y<=0; y++)
//	for (int x=0; x<=0; x++) {
//

		par_cur[0] = par_min[0] + (double)x*inc;
		par_cur[1] = par_min[1] + (double)y*inc;
		par_cur[2] = par_min[2] + (double)z*inc;

		try {obj_val = obj_val_at(par_cur);}
		catch (Range_Fail) {throw Range_Fail();}

// output subvol values, code check, use subvol_geom cube, need basin_radius on
// raw: cube root of file_size/8, 64-bit real, little endian
//		if (n==0 && z==0 && y==0 && x==0) {
//				std::ofstream out("tar_subvol_echo.raw", std::ofstream::out | std::ofstream::binary);
//				out.write((char*)&tar_subvol[0], tar_subvol.size()*sizeof(double));
//				out.close();
//		}
//

		// objective function mapping
		if (out_as_raw) 
			obj_vals[count] = obj_val;
		//

		if (obj_val < obj_min) {
			obj_min = obj_val;
			min_x = x;
			min_y = y;
			min_z = z;
		}

		if (obj_val > obj_max)		// for scaling output
			obj_max = obj_val;

		count += 1;
	}

	par_min[0] += (double)min_x*inc;
	par_min[1] += (double)min_y*inc;
	par_min[2] += (double)min_z*inc;

	// return;

	// objective function mapping, need basin_radius on
	// optional output block, write as a raw 8-bit image volume

	if (out_as_raw) 
	{
		int ncp = 2*num_inc+1;

		std::ofstream ofs;
		ofs.open("global_echo.raw", std::ofstream::out | std::ofstream::binary);
		if (!ofs)std::cout <<"\n-> Can't open " << "global_echo.raw" << "\n\n";

		char *row_seg;
		row_seg = new char [ncp];	// storage space for bytes write

		count = 0;
		for (int i=0; i<ncp; i++)
		for (int j=0; j<ncp; j++)
		{
			for (int k=0; k<ncp; k++) {
				double scaled = 255*((obj_vals[count]-obj_min)/(obj_max-obj_min));
				row_seg[k] = (char)scaled;
				count += 1;
				}

				ofs.write (row_seg, ncp);	// write a row
				}
		ofs.close();
	}
	return;

}
/******************************************************************************/
void Search::map_objective_function(int map_id, double half_range, int num_each_dim) {

	std::cout << std::endl << "* objective function mapping point " << map_id << ", span = " << 2.0*half_range << " voxels, sampling points = " << num_each_dim << std::endl;

	double inc = (2.0*half_range)/num_each_dim;
	int ndof = 3;
	double obj_val = 0.0;
	int total_num = num_each_dim*num_each_dim*num_each_dim;

	std::vector<double> par_cur(ndof,0.0);
	std::vector<double> obj_vals(total_num,0.0);

	double obj_min = std::numeric_limits<double>::max();	// for scaling output
	double obj_max = std::numeric_limits<double>::min();	// for scaling output

	double total_num_pts = num_each_dim * num_each_dim * num_each_dim;
	double status_percent_inc = 10;

	int count = 0;
	double status_percent_current = status_percent_inc;
	int past_current_limit = 0;
	for (int i=0; i<num_each_dim; i++) {
		double delx = -half_range + i*inc;
		for (int j=0; j<num_each_dim; j++) {
			double dely = -half_range + j*inc;
			for (int k=0; k<num_each_dim; k++) {
				double delz = -half_range + k*inc;

				par_cur[0] = par_min[0] + delx;
				par_cur[1] = par_min[1] + dely;
				par_cur[2] = par_min[2] + delz;

				try {obj_val = obj_val_at(par_cur);}
				catch (Range_Fail) {throw Range_Fail();}

				obj_vals[count] = obj_val;

				if (obj_val < obj_min) obj_min = obj_val;
				if (obj_val > obj_max) obj_max = obj_val;

				if (count > total_num_pts * (status_percent_current/100)) {
					past_current_limit += 1;
					if (past_current_limit == 1) {
						std::cout << "map progress: " << status_percent_current  << "% " << std::endl;
						status_percent_current = status_percent_current + status_percent_inc;
						past_current_limit = 0;
					}
				}
				count += 1;
			}
		}
	}

	// write as raw image file
	int ncp = num_each_dim;

	std::ofstream ofs;
	ofs.open("global_echo.raw", std::ofstream::out | std::ofstream::binary);
	if (!ofs)std::cout <<"\n-> Can't open " << "global_echo.raw" << "\n\n";

	char *row_seg;
	row_seg = new char [ncp];	// storage space for bytes write

	count = 0;
	for (int i=0; i<ncp; i++) {
		for (int j=0; j<ncp; j++) {
			for (int k=0; k<ncp; k++) {

					double scaled = 255*((obj_vals[count]-obj_min)/(obj_max-obj_min));
					row_seg[k] = (char)scaled;

					count += 1;
				}
				ofs.write (row_seg, ncp);	// write an int scaled row
		}
	}
		ofs.close();
	
	return;
}
/******************************************************************************/

void Search::random_global(double displ_max, double basin_radius)
// figure out the number of points implied by a translation grid search of the same scope
// then search a random set of that many points distributed through the same spatial region
// could be translation only or add rotation (adjust point number)
{
	// check for opt out of coarse search
	if (basin_radius == 0.0) return;

	int num_inc = displ_max/basin_radius;
	double inc = displ_max/num_inc;
	int num_pts = (2*num_inc+1)*(2*num_inc+1)*(2*num_inc+1);

	int ndof = 3;	// just search global with translation dof's

	std::vector<double> par_cur(ndof,0.0);

	srand (time(NULL));
	double dbl_rand_max = RAND_MAX;

	double obj_min = std::numeric_limits<double>::max();
	double obj_val;
	double min_x;
	double min_y;
	double min_z;

	for (int i=0; i<num_pts; i++) {

		double rel_x = displ_max*((2*rand())/dbl_rand_max);
		double rel_y = displ_max*((2*rand())/dbl_rand_max);
		double rel_z = displ_max*((2*rand())/dbl_rand_max);

		par_cur[0] = par_min[0] + rel_x;
		par_cur[1] = par_min[1] + rel_y;
		par_cur[2] = par_min[2] + rel_z;

		try {obj_val = obj_val_at(par_cur);}
		catch (Range_Fail) {throw Range_Fail();}

		if (obj_val < obj_min) {
			obj_min = obj_val;
			min_x = rel_x;
			min_y = rel_y;
			min_z = rel_z;
		}

	}

	par_min[0] += min_x;
	par_min[1] += min_y;
	par_min[2] += min_z;

	return;

}

/******************************************************************************/

#if defined(_WIN32) || defined(__WIN32__)
/**/
#else
std::ostream& operator<<(std::ostream &strm, const Search &a) {
	RunControl * run = a.rc;

	// SSD, ZSSD, NSSD, ZNSSD
	std::string objfun;
	switch (run->obj_fcn) {
		case SSD:
			objfun = "SSD";
			break;
		case ZSSD:
			objfun = "ZSSD";
			break;
		case NSSD:
			objfun = "NSSD";
			break;
		case ZNSSD:
			objfun = "ZNSSD";
			break;
	}

	std::string inttyp;
	switch (run->int_typ) {
		case trilinear:
			inttyp = "trilinear";
			break;
		case tricubic:
			inttyp = "tricubic";
			break;
		case tri_bspline_3:
			inttyp = "tri_bspline_3";
			break;
		case tri_bspline_5:
			inttyp = "tri_bspline_5";
			break;
		case tri_bspline_7:
			inttyp = "tri_bspline_7";
			break;
	}

	std::string geotyp;
	switch (run->sub_geo) {
		case cube:
			geotyp = "cube";
			break;
		case sphere:
			geotyp = "sphere";
			break;
	}

	return strm << "Search settings: (" << std::endl <<
		"bytes_per " << a.bytes_per << std::endl <<
		"image volume size " << run->vol_wide << " " << run->vol_high << " " << run->vol_tall << std::endl <<
		"subvol_geom " << geotyp << std::endl <<
		"subvol_size " << 2*a.subv_rad << std::endl <<
		"subvol_npts " << a.subv_num << std::endl <<
		"interp_type " << inttyp << std::endl <<
		"obj_fun " << objfun << std::endl <<
		"num_srch_dof " << run->num_srch_dof << std::endl <<
		"step_max " << run->step_max << std::endl <<
		"cost_tol (obj) " << std::scientific << std::setprecision(1) << a.rc->cost_tol << std::endl <<
		"step_tol (par) " << std::scientific << std::setprecision(1) << a.rc->step_tol << std::endl <<
		"max_iter " << std::fixed << std::setprecision(0) << a.rc->max_iter << std::endl <<
		")";
}
#endif

