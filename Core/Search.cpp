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

	if (rc->obj_fcn == SSD) {
		obj_fcn = &obj_SSD;
		obj_fcn_res = &obj_SSD;
	}
	if (rc->obj_fcn == ZSSD) {
		obj_fcn = &obj_ZSSD;
		obj_fcn_res = &obj_ZSSD;
	}
	if (rc->obj_fcn == NSSD) {
		obj_fcn = &obj_NSSD;
		obj_fcn_res = &obj_NSSD;
	}
	if (rc->obj_fcn == ZNSSD) {
		obj_fcn = &obj_ZNSSD;
		obj_fcn_res = &obj_ZNSSD;
	}

	// create a box with the full dimensions of the image voxel volumes
	Point vox_box_min(0.0, 0.0, 0.0);
	Point vox_box_max(rc->vol_wide, rc->vol_high, rc->vol_tall);
	vox_box = new BoundBox(vox_box_min, vox_box_max);

	// create reference and correlate (target) subvolume vectors of interpolated voxel data, init to 0.0
	ref_subvol = std::vector<double>(subv_num,0.0);
	tar_subvol = std::vector<double>(subv_num,0.0);

	// set convergence criteria
	obj_tol = 0.000001;		// objective function change threshold that defines convergence
	pos_tol = 0.01;			// parameter vector displcement mag change threshold that defines convergence
	maxit = 20;				// max iterations allowed

	// optimization result storage 
	par_min = std::vector<double>(rc->num_srch_dof,0.0);	// note size, rc->num_srch_dof

	// set-up a single fcld + disp_max sized interp region

	// create a subvolume size box, then expand by the disp_max parameter (change to opt_translate_max?)
	// position is initially at the corner of the vox_box
	// note that subv_rad applies to both spheres and cubes, and is a half-width
	Point est_box_nom_min = Point(0.0, 0.0, 0.0);
	Point est_box_nom_max = Point(2*subv_rad*rc->subvol_aspect[0], 2*subv_rad*rc->subvol_aspect[1], 2*subv_rad*rc->subvol_aspect[2]);
	est_box_nom = new BoundBox(est_box_nom_min, est_box_nom_max);
	est_box_nom->grow_by(rc->disp_max);

	// create interpolator of suitable capacity.
	//
	// The two paths below both end up with the same net safety margin (1
	// voxel) beyond disp_max on act_box, but they get there differently:
	//
	//  - legacy path (trilinear/Lekien tricubic): the single-argument
	//    Interpolate constructor always reserves a fixed frame of 1.0, so the
	//    "+2.0" grown here becomes a net +1.0 margin once Interpolate shrinks
	//    act_box by that fixed frame.
	//
	//  - B-spline path: the two-argument constructor reserves whatever halo
	//    the chosen order actually needs (2 voxels for cubic, 3 for quintic,
	//    4 for septic) automatically -- so growing by a flat "+2.0" here (as
	//    the legacy path does) would leave a cubic B-spline object with ZERO
	//    net margin beyond disp_max (2.0 grown here - 2.0 reserved internally),
	//    which is exactly the halo/margin shortfall you were chasing earlier.
	//    Instead we grow by just the desired net safety margin (1.0) and let
	//    Interpolate's own constructor reserve the halo on top of that.

	if (rc->bspline == true)
	{
		const int bspline_order_cfg = rc->bspline_order;
		est_box_nom->grow_by(1.0);	// net safety margin beyond disp_max, same convention as the legacy path below
		// this is the two argument bspline constructor
		interp = new Interpolate(est_box_nom, bspline_order_cfg);	
	}
	else
	{
		// compensate for derivatives in interpolator (fixed frame=1.0 internally, so this nets +1.0 margin)
		est_box_nom->grow_by(2.0);
		// this is the tricubic constructor
		interp = new Interpolate(est_box_nom);
	}
}
/******************************************************************************/

Search::~Search()
{
	delete interp;
	// fcld new/delete from within process_point

	delete vox_box;
	delete est_box_nom;

}
/******************************************************************************/
void Search::process_point(int t, int n, bool map_flag, int map_id, DataCloud *srch_data)
{
	// n is the index of the current search point
	Point srch_pt = srch_data->points[n];

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
	search_pt_setup(srch_pt, neigh_res);

	// *** special cases

	// trap point for objective function mapping
	if (map_flag && (srch_data->labels[n] == map_id)) {
		std::cout << std::endl << "** mapping point with label " << map_id << std::endl;
		// mapping range is taken from the disp_max value in the dvc input file
		double half_range = rc->disp_max;
		int num_each_dim = 100;		// hardcode this for now, could be a command line input in future
		map_objective_function(map_id, half_range, num_each_dim);
	}

	// coarse search step, this is probably going to phase out, except perhaps for optimization start refinement in special cases
	// basin_radius = 0.0 in the input file signals no coarse search step
	if (rc->basin_radius > 0.0) {trgrid_global(rc->disp_max, rc->basin_radius, n, false);}
	// random search is also reserved for optimization start refinement in special cases, not triggered in current code configuration
	//	random_global(rc->disp_max, rc->basin_radius);

/*******************/

	// L-M optimization, currently using pure QN steps (no lambda tuning)
	// not yet reporting on convg or range failures



/*
Several ting to do here.
1. The obj_val_at below should be eliminated. It duplicates the final call in min_Lev_Mar. 
	Reducing 1 obj_val_at by 1 is a big win with iterations numbers genreally low (3,4,5). 
	It's not correct 
2. Need to build try/catch in the iterations in case an out of box param vector is tried.
	Currently any Range_Fail being thrown must be fro this final check, but it should be at each obj eval in min_Lev_Mar
	Then just keep obj_min aready avaialble and utilized in Search updated. 
*/





	int ndof = par_min.size();
	std::vector<double> jump(ndof, 0.0);

	jump = min_Lev_Mar(par_min, srch_data);

	// need to change this, mi_Lav_Mar returning a jump even if maxit reached, that should be a Convg_Fail

	for (int i=0; i<ndof; i++) {
		par_min[i] = jump[i];
	}

	//
	//
/********************/

	// update status of Search members

	try {obj_min = obj_val_at(par_min);}
	catch (Range_Fail) {throw Range_Fail();}

	delete fcld;
	throw Point_Good();
}
/******************************************************************************/
std::vector<double> Search::min_Lev_Mar(const std::vector<double> &start, DataCloud *srch_data)
// obj_tol compares with change in the objective function at each iteration
// pos_tol compares with position change at each iteration
{
	int npts = subv_num;
	int ndof = start.size();
	std::vector<double> jump(ndof, 0.0);
	Iter_Stats point_stats;

	Eigen::VectorXd e = Eigen::VectorXd(npts);
	Eigen::MatrixXd J = Eigen::MatrixXd(npts,ndof);
	Eigen::MatrixXd JTJ = Eigen::MatrixXd(ndof,ndof);
	Eigen::VectorXd JTe = Eigen::VectorXd(ndof);
	Eigen::VectorXd update = Eigen::VectorXd(ndof);

	// jump contains the updated parameter vector as optimization proceeds
	for (int i=0; i<ndof; i++) {
			jump[i] = start[i];
	}

	double obj_old = 0.0;

	// track number of iterations, start with 1 for nits updated within the convergence check conditional
	int nits = 1;
	int obj_nits = 1;
	int pos_nits = 1;

	double del_obj;		// change on objective function value from prior it
	double del_pos;		// change in position from prior it

	for (int i=0; i<maxit; i++) {

		double obj = LM_prep_at(jump, e, J);

		if (i==0) {
			iter_stats.obj_beg = obj;
			iter_stats.pos_beg.x = jump[0];
			iter_stats.pos_beg.y = jump[1];
			iter_stats.pos_beg.z = jump[2];
		}

		// convergence check
		if (i>0) {
			del_obj = fabs(obj - obj_old);
			del_pos = sqrt(update(0)*update(0) + update(1)*update(1) + update(2)*update(2));

			nits += 1;
			if (del_obj > obj_tol) {
				obj_nits += 1;
			}
			if (del_pos > pos_tol) {
				pos_nits += 1;
			}

			// point has converged for either obj or pos criteria
			if ((del_obj <= obj_tol) || (del_pos <= pos_tol)) {
				iter_stats.nits = nits;
				iter_stats.obj_nits = obj_nits;
				iter_stats.pos_nits = pos_nits;

				iter_stats.obj_update_last_it = del_obj;
				iter_stats.pos_update_last_it = del_pos;
				iter_stats.obj_end = obj;
				
				iter_stats.pos_end.x = jump[0];
				iter_stats.pos_end.y = jump[1];
				iter_stats.pos_end.z = jump[2];
				break;
			}
		}

		JTJ = J.transpose()*J;
		JTe = J.transpose()*e;
		update = JTJ.colPivHouseholderQr().solve(-JTe);

		for (int j=0; j<ndof; j++) {
			jump[j] += update(j);
		}

		obj_old = obj;

		if (i==maxit) {
			iter_stats.nits = maxit;
			iter_stats.obj_nits = maxit;
			iter_stats.pos_nits = maxit;

			iter_stats.obj_update_last_it = del_obj;
			iter_stats.pos_update_last_it = del_pos;
			iter_stats.obj_end = obj;

			iter_stats.pos_end.x = jump[0];
			iter_stats.pos_end.y = jump[1];
			iter_stats.pos_end.z = jump[2];
		}
	}

	return jump;
}
/******************************************************************************/
void Search::search_pt_setup(Point srch_pt, std::vector<ResultRecord> &neigh_res)
{
	// Interpolation kernels are established during this stage. 
	//
	// A kernel is first developed/used for one interpolation of reference volume data
	// to establish the ref_subvol vector of interpolated sampling point values. 
	//
	// A kernel is then developed and used for the multiple interpolations of correlate volume data
	// to establish tar_subvolume sampling point values as needed for optimization. 
	//
	// tri_lin, tri_cub_Lek, and tri_bspline utilize the same basic foundation 
	// of voxel values and derivatives at the voxel centers, stored in a matrix for fast mult/sum.
	//
	// For tri_lin/tri_cub_leK: interp->kernels (which then calls interp->kernels_derivs).
	//		This loads voxel data directly, then calculates derivatives at the voxel locations. 
	//		tri_lin require no further kernel development.
	//		tri_cub_leK calls further "on demand" kernel development as needed for individual sampling points.
	//
	// For tri_bspline: interp->kernels followed by interp->kernels_bspline
	//		This reads voxel data, finds voxel derivatives, then calculates the bspline-specific coefficientc. 
	//		This assumes interp = new Interpolate(est_box_nom, bspline_order_cfg) has been invoked with order set. 

	// set kernels/kernels_derivs and interpolate the reference volume
	interp->center_on(srch_pt);
	interp->kernels(rc->ref_fname, vox_box, bytes_per, rc->vol_endian, rc->vol_hdr_lngth);

	if (rc->int_typ == trilinear) {
		interp->tri_lin(fcld->stable->ptvect, fcld->stable->bbox(), ref_subvol);
	}
	if (rc->int_typ == tricubic) {
		interp->tri_cub_Lek(fcld->stable->ptvect, fcld->stable->bbox(), ref_subvol);
	}
	if (rc->bspline == true) {
		interp->kernels_bspline();
		interp->tri_bspline(fcld->stable->ptvect, fcld->stable->bbox(), ref_subvol);
	}


	// re-set kernels for the moving cloud to prepare for subsequent interp calls
	starting_param(srch_pt, neigh_res);
	Point offset_pt = srch_pt;
	offset_pt.move_by(par_min[0], par_min[1], par_min[2]);
	interp->center_on(offset_pt);

	// this covers tri_lin/tri_cub_leK and partially grenerates tri_bspline
	interp->kernels(rc->cor_fname, vox_box, bytes_per, rc->vol_endian, (unsigned int)rc->vol_hdr_lngth);

	// this completes kernel development when tri_bspline options are active
	if (rc->bspline == true) {
		interp->kernels_bspline();
	}

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
	// note: with no past points found and no rigid_trans set initial becomes zero

	if (good != 0)
	{
		par_min[0] =  past_srch_x;
		par_min[1] =  past_srch_y;
		par_min[2] =  past_srch_z;
	} else
	{
		par_min[0] =  rc->rigid_trans[0];
		par_min[1] =  rc->rigid_trans[1];
		par_min[2] =  rc->rigid_trans[2];
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
		try {interp->tri_lin(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}}

	if (rc->int_typ == tricubic) {
		try {interp->tri_cub_Lek(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}}

	if (rc->bspline == true) {
		// kernels_bspline() is NOT called here -- it's primed once in
		// search_pt_setup() right after the moving-cloud kernels() load, and
		// stays valid (bsp_valid) across every obj_val_at() call in this
		// search point's optimization loop, since fcld->affine_to() only
		// moves query points, it never reloads voxel data.
		try {interp->tri_bspline(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}}

	double obj_val = obj_fcn(ref_subvol, tar_subvol);

	return obj_val;
}
/******************************************************************************/
double Search::obj_val_at(const std::vector<double> x, std::vector<double> &residual)	// this version returns residual vector as well
{
	fcld->affine_to(x, x.size());

	if (rc->int_typ == trilinear) {
		try {interp->tri_lin(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}}

	if (rc->int_typ == tricubic) {
		try {interp->tri_cub_Lek(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}}

	if (rc->bspline == true) {
		// see obj_val_at(x) above -- kernels_bspline() is primed once in
		// search_pt_setup(), not on every call here.
		try {interp->tri_bspline(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
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
	try {interp->tri_bspline_grad(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol, dfdx, dfdy, dfdz);}
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
			objfun = std::string("SSD");
			break;
		case ZSSD:
			objfun = std::string("ZSSD");
			break;
		case NSSD:
			objfun = std::string("NSSD");
			break;
		case ZNSSD:
			objfun = std::string("ZNSSD");
			break;
	}

	std::string inttyp;
	switch (run->int_typ) {
		case trilinear:
			inttyp = std::string("trilinear");
			break;
		case tricubic:
			inttyp = std::string("tricubic");
			break;
		case tri_bspline_3:
			inttyp = std::string("tri_bspline_3");
			break;
		case tri_bspline_5:
			inttyp = std::string("tri_bspline_5");
			break;
		case tri_bspline_7:
			inttyp = std::string("tri_bspline_7");
			break;
	}

	std::string geotyp;
	switch (run->sub_geo) {
		case cube:
			geotyp = std::string("cube");
			break;
		case sphere:
			geotyp = std::string("sphere");
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
		"disp_max " << run->disp_max << std::endl <<
		")";
}
#endif

