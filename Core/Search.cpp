
#include "Search.h"

/******************************************************************************/
Search::Search(RunControl *run)
{
	rc = run;

	bytes_per = run->vol_bit_depth/8;
	subv_rad = (run->subvol_size)/2.0;	// easier to use than subvol_size
	if (run->sub_geo == cube) subv_num = pow(ceil(pow((double)run->subvol_npts,1.0/3.0)),3.0);
	if (run->sub_geo == sphere) subv_num = run->subvol_npts;

	// establish pointer to function set in input file
	if (run->obj_fcn == SAD) {
		obj_fcn = &obj_SAD;
		obj_fcn_res = &obj_SAD;
	}
	if (run->obj_fcn == SSD) {
		obj_fcn = &obj_SSD;
		obj_fcn_res = &obj_SSD;
	}
	if (run->obj_fcn == ZSSD) {
		obj_fcn = &obj_ZSSD;
		obj_fcn_res = &obj_ZSSD;
	}
	if (run->obj_fcn == NSSD) {
		obj_fcn = &obj_NSSD;
		obj_fcn_res = &obj_NSSD;
	}
	if (run->obj_fcn == ZNSSD) {
		obj_fcn = &obj_ZNSSD;
		obj_fcn_res = &obj_ZNSSD;
	}

	Point vox_box_min(0.0, 0.0, 0.0);
	Point vox_box_max(run->vol_wide, run->vol_high, run->vol_tall);
	vox_box = new BoundBox(vox_box_min, vox_box_max);

	ref_subvol = std::vector<double>(subv_num,0.0);
	tar_subvol = std::vector<double>(subv_num,0.0);

	par_min = std::vector<double>(rc->num_srch_dof,0.0);	// note size, rc->num_srch_dof

	// set-up a single fcld + disp_max sized interp region

	Point est_box_nom_min = Point(0.0, 0.0, 0.0);

	Point est_box_nom_max = Point(2*subv_rad*rc->subvol_aspect[0], 2*subv_rad*rc->subvol_aspect[1], 2*subv_rad*rc->subvol_aspect[2]);

	est_box_nom = new BoundBox(est_box_nom_min, est_box_nom_max);

	est_box_nom->grow_by(rc->disp_max);

	// compensate for derivatives in interpolator

	est_box_nom->grow_by(2.0);

	// create interpolator of suitable capacity

	interp = new Interpolate(est_box_nom);

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
void Search::process_point(int t, int n, DataCloud *srch_data)
{
	// n is the index of the current search point
	Point srch_pt = srch_data->points[n];

	if (rc->sub_geo == sphere)
		fcld = new FloatingCloud(srch_pt, subv_rad, subv_num, rc->subvol_aspect[0], rc->subvol_aspect[1], rc->subvol_aspect[2]);

	if (rc->sub_geo == cube) {
		Point sub_min = srch_pt;
		sub_min.move_by(-subv_rad*rc->subvol_aspect[0], -subv_rad*rc->subvol_aspect[1], -subv_rad*rc->subvol_aspect[2]);
		Point sub_max = srch_pt;
		sub_max.move_by(subv_rad*rc->subvol_aspect[0], subv_rad*rc->subvol_aspect[1], subv_rad*rc->subvol_aspect[2]);
		int grid_inc = ceil(pow((double)subv_num,1.0/3.0));
		fcld = new FloatingCloud(sub_min, sub_max, grid_inc, grid_inc, grid_inc);
	}

	// vector of ResultRecord for the neighborhood of this search point
	std::vector<ResultRecord> neigh_res;
	for (int i=0; i<srch_data->neigh[n].size(); i++)
		neigh_res.push_back(srch_data->results[t][srch_data->neigh[n][i]]);

	// this sets par_min to the starting point estimate through starting_param call
	search_pt_setup(srch_pt, neigh_res);

	// coarse search step
	// this is probably going to phase out, except perhaps for global start
	// also useful for objective function mapping
	trgrid_global(rc->disp_max, rc->basin_radius, n);
	//	random_global(rc->disp_max, rc->basin_radius);

	// L-M optimization, currently using pure QN steps (no lambda tuning)
	// not yet reporting on convg or range failures
	int ndof = par_min.size();
	std::vector<double> jump(ndof, 0.0);
	jump = min_Lev_Mar(par_min, 0.000001, 0.01);
	for (int i=0; i<ndof; i++) {
		par_min[i] = jump[i];
	}
	//
	//

/*
	// basic N-M process
	//
	//
	try
	{
		// look_with(amoeba, 3, 0.00001); // does disp only search first for all searches
		// sequence the 6 and 12 dof searches
		//if (rc->num_srch_dof > 3) look_with(amoeba, 6, 0.0000001);
		//if (rc->num_srch_dof > 6) look_with(amoeba,12, 0.0000001);


		// do all standalone
		if (rc->num_srch_dof == 3) look_with(amoeba, 3, 0.00001);
		if (rc->num_srch_dof == 6) look_with(amoeba, 6, 0.0000001);
		if (rc->num_srch_dof == 12) look_with(amoeba,12, 0.0000001);

	}
	catch (Convg_Fail)
	{
		delete fcld;
		throw Convg_Fail();
	}
	catch (Range_Fail)
	{
		delete fcld;
		throw Range_Fail();
	}
	//
	//
	//
*/


	// update status of Search members
	try {obj_min = obj_val_at(par_min);}
	catch (Range_Fail) {throw Range_Fail();}

	delete fcld;
	throw Point_Good();
}
/******************************************************************************/
std::vector<double> Search::min_Lev_Mar(const std::vector<double> &start, const double obj_tol, const double mag_tol)
// obj_tol compares with change in the objective function at each iteration
// mag_tol compares with displacement magnitude change at each iteration
{
	int npts = subv_num;
	int ndof = start.size();
	std::vector<double> jump(ndof, 0.0);

	Eigen::VectorXd e = Eigen::VectorXd(npts);
	Eigen::MatrixXd J = Eigen::MatrixXd(npts,ndof);
	Eigen::MatrixXd JTJ = Eigen::MatrixXd(ndof,ndof);
	Eigen::VectorXd JTe = Eigen::VectorXd(ndof);
	Eigen::VectorXd update = Eigen::VectorXd(ndof);

	for (int i=0; i<ndof; i++) {
			jump[i] = start[i];
	}

//	std::cout << "\n" << std::setprecision(12);
//	std::cout << "p: " << jump[0] << "\t" << jump[1] << "\t" << jump[2] << "\n";

	double obj_old = 0.0;
	int maxit = 20;
	int nits = 0;

	for (int i=0; i<maxit; i++) {
		double obj = LM_prep_at(jump, e, J);

		if (i>0) {
			double del_obj = fabs(obj - obj_old);
			double del_mag = sqrt(update(0)*update(0) + update(1)*update(1) + update(2)*update(2));
//			std::cout << "n: " << i << "\t obj_del = " << del_obj << "\t del_dis = " << del_mag << "\t p: " << jump[0] << "\t" << jump[1] << "\t" << jump[2] << "\n";
			if ((del_obj < obj_tol) || (del_mag < mag_tol)) {
				nits = i;
				break;
			}
		}

		JTJ = J.transpose()*J;
		JTe = J.transpose()*e;
		update = JTJ.colPivHouseholderQr().solve(-JTe);

		for (int i=0; i<ndof; i++) {
				jump[i] += update(i);
		}

		obj_old = obj;
	}

//	std::cout << std::setprecision(6);

	return jump;
}
/******************************************************************************/
void Search::search_pt_setup(Point srch_pt, std::vector<ResultRecord> &neigh_res)
{
	// set kernels and interpolate the reference volume

	interp->center_on(srch_pt);

	interp->kernels(rc->ref_fname, vox_box, bytes_per, rc->vol_endian, rc->vol_hdr_lngth);

	if (rc->int_typ == nearest) interp->nearest(fcld->stable->ptvect, fcld->stable->bbox(), ref_subvol);
	if (rc->int_typ == trilinear) interp->tri_lin(fcld->stable->ptvect, fcld->stable->bbox(), ref_subvol);
	if (rc->int_typ == tricubic) interp->tri_cub_Lek(fcld->stable->ptvect, fcld->stable->bbox(), ref_subvol);

	// re-set kernels for the moving cloud

	// how to integrate neigh results? other starting point refinements?
	// what if the best neigh result differs substantially from rigid_trans or other estimates?

	starting_param(srch_pt, neigh_res);

	Point offset_pt = srch_pt;

	offset_pt.move_by(par_min[0], par_min[1], par_min[2]);

	interp->center_on(offset_pt);

	interp->kernels(rc->cor_fname, vox_box, bytes_per, rc->vol_endian, (unsigned int)rc->vol_hdr_lngth);

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
void Search::look_with(Search_Type method, int ndof, double ftol)
{
	// initial set
//	double del_tra = 0.1;		// for translational dof's
//	double del_rot = 0.001;		// for rotational dof's
//	double del_str = 0.001;		// for strain dof's

	// ran twice as fast in "old" test case
//	double del_tra = 0.1;		// for translational dof's
//	double del_rot = 0.01;		// for rotational dof's
//	double del_str = 0.001;		// for strain dof's

	// calculate rot and str delta's balanced with the translation delta
	// 0.75 is distance from center of subvolume to a representative point
	// del_tra of 0.2 comes from an initial test problem convergence study

	double del_tra = 0.2;
	double del_rot = del_tra;
	// double del_rot = del_tra/(0.75*subv_rad); // very consertative w/ rot
	double del_str = del_rot;

	std::vector<double> pmin(ndof, 0.0);; //
	std::vector<double> dels(ndof, 0.0);; //

	for (int i=0; i<ndof; i++)
	{
		pmin[i] = par_min[i];

		if (i< 3)
		{
			dels[i] = del_tra;
		}
		else if (i< 6)
		{
			dels[i] = del_rot;
		}
		else if (i<12)
		{
			dels[i] = del_str;
		}
	}

	// custom Nelder-Mead using the functor, not working
/*	if (method == amoeba) {
		Min_Ftor_new simplex_ftor(int_typ,obj_typ,ndof,fcld,interp,ref_subvol);	// this is OK

		Simplex simplex;// this is OK

		// this line throws an "undefined"
//		pmin_new = simplex.minimize(pmin_new, dels_new, simplex_ftor, ftol);

	}
*/

	// custom Nelder-Mead using Search internal objective function evaluation
	if (method == amoeba)
	{
		try {pmin = min_Nelder_Mead(pmin, dels, ftol);}
		catch (Range_Fail) {throw Range_Fail();}
		catch (Convg_Fail) {throw Convg_Fail();}
	}

	for (int i=0; i<ndof; i++)
	{
		par_min[i] = pmin[i];
	}

}
/******************************************************************************/
double Search::obj_val_at(const std::vector<double> x)	// this version uses nominals set at Search construct
{
	fcld->affine_to(x, x.size());

	if (rc->int_typ == nearest) {
		try {interp->nearest(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}}

	if (rc->int_typ == trilinear) {
		try {interp->tri_lin(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}}

	if (rc->int_typ == tricubic) {
		try {interp->tri_cub_Lek(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}}

	double obj_val = obj_fcn(ref_subvol, tar_subvol);

	return obj_val;
}
/******************************************************************************/
double Search::obj_val_at(const std::vector<double> x, std::vector<double> &residual)	// this version returns residual vector as well
{
	fcld->affine_to(x, x.size());

	if (rc->int_typ == nearest) {
		try {interp->nearest(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}}

	if (rc->int_typ == trilinear) {
		try {interp->tri_lin(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}}

	if (rc->int_typ == tricubic) {
		try {interp->tri_cub_Lek(fcld->moving->ptvect, fcld->moving->bbox(), tar_subvol);}
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
void Search::Jacobian_at (const std::vector<double> a, std::vector< std::vector<double> > &J)
// a[ndof]
// J[npts][ndof]
// simple forward diff
{
	int npts = J.size();
	int ndof = J[0].size();

	std::vector<double> base_res(npts, 0.0);
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
std::vector<double> Search::obj_grad_at(const std::vector<double> x)
{
	int ndof = x.size();
	std::vector<double> x_step = x;
	std::vector<double> gradient(ndof, 0.0);

	double h = 1E-8;		// checked e-4 to e-12, all similar to several places
	double add = 0.0;
	double sub = 0.0;

	// output obj values to test derivatives
	std::cout << "\n\n" << std::setprecision(20);
	double obj = 0.0;
	x_step = x;

	x_step[0] -= 2.0*h;
	obj = obj_val_at(x_step);
	std::cout << "\n" << obj << "\t" << x_step[0] << "\t" << x_step[1] << "\t" << x_step[2] << "\n";

	x_step[0] += h;
	obj = obj_val_at(x_step);
	std::cout << "\n" << obj << "\t" << x_step[0] << "\t" << x_step[1] << "\t" << x_step[2] << "\n";

	x_step[0] += h;
	obj = obj_val_at(x_step);
	std::cout << "\n" << obj << "\t" << x_step[0] << "\t" << x_step[1] << "\t" << x_step[2] << "\n";

	x_step[0] += h;
	obj = obj_val_at(x_step);
	std::cout << "\n" << obj << "\t" << x_step[0] << "\t" << x_step[1] << "\t" << x_step[2] << "\n";

	x_step[0] += h;
	obj = obj_val_at(x_step);
	std::cout << "\n" << obj << "\t" << x_step[0] << "\t" << x_step[1] << "\t" << x_step[2] << "\n";

	std::cout << "\n" << std::setprecision(6);
	x_step = x;
	//

	// this is the actual gradient calc
	for (int i=0; i<ndof; i++) {
		x_step[i] += h;
		add = obj_val_at(x_step);
		x_step[i] -= 2.0*h;
		sub = obj_val_at(x_step);
		gradient[i] = (add - sub)/(2.0*h);
		x_step[i] = x[i];
	}

	std::cout << "\n" << std::setprecision(20);
	std::cout << gradient[0] << "\t" << gradient[1] << "\t" << gradient[2] << "\n";
	std::cout << "\n" << std::setprecision(6);

	return gradient;
}
/******************************************************************************/
std::vector< std::vector<double> > Search::obj_Hess_at(const std::vector<double> x)
{
	// old-school Hessian by forward differences

	int ndof = x.size();
	std::vector<double> x_step = x;
	double h = 1E-8;
	double f1 = 0.0;
	double f2 = 0.0;
	double f3 = 0.0;
	double f4 = 0.0;

	std::vector< std::vector<double> > hessian;
	hessian.resize(ndof);
	for (int i=0; i<ndof; i++) {
		hessian[i].resize(ndof, 0.0);
	}

	std::vector< std::vector<double> > hessian_T = hessian;

	f4 = obj_val_at(x_step);
	// for (int i=0; i<ndof; i++) {
	for (int i=0; i<=0; i++) {
		x_step = x;
		x_step[i] += h;
		f2 = obj_val_at(x_step);
		//for (int j=0; j<ndof; j++) {
		for (int j=0; j<=0; j++) {
			x_step = x;
			x_step[j] += h;
			f3 = obj_val_at(x_step);
			x_step = x;
			x_step[i] += h;
			x_step[j] += h;
			f1 = obj_val_at(x_step);

			hessian[i][j] = (f1 - f2 - f3 + f4)/(h*h);
			hessian_T[j][i] = hessian[i][j];
		}
	}

	std::cout << "\n" << std::setprecision(20);
	std::cout << f1 << "\t" << f2 << "\t"<< f3 << "\t"<< f4 << "\n";

	std::cout << "\n" << std::setprecision(20);
	std::cout << hessian[0][0] << "\t" << hessian[0][1] << "\t" << hessian[0][2] << "\n";
	std::cout << "\n" << std::setprecision(6);


	// replace H with 0.5*(H + HT) to force symmetry
	/*
	std::cout << "\n";
	for (int i=0; i<ndof; i++) {
		for (int j=0; j<ndof; j++) {
			hessian[i][j] = 0.5*(hessian[i][j] + hessian_T[i][j]);

			std::cout << hessian[i][j] << "\t";

		}
		std::cout << "\n";
	}
*/

	return hessian;
}
/******************************************************************************/
std::vector<double> Search::min_Nelder_Mead(std::vector<double> &start, std::vector<double> &dels, double conv_tol)
{
	const int MAX_FUNC = 2000;		// max allowed objective function evaluations
	const double NOT_ZERO = 1.0e-10;	// prevent divide by zero

	int ndof = start.size();	// dimensionality of the parameter space
	int nvrt = ndof + 1;		// number of vertices in the simplex
	int num_func = 0;		// count num of function evals, for convergence checking

	double ALPHA =  1.0;	// >0, typically 1.0;		reflection scaling
	double GAMMA =  2.0;	// >alpha, typically 2.0;	expansion scaling
	double DELTA = -0.5;	// mag >0, <=0.5, typ 0.5;	contract worst toward centroid
	double OMEGA =  0.5;	// >0, <=0.5, typ = delta;	contract all toward best
//	double NUDGE =  0.1;	// bet 0 & 1, typ = 0.1;	nudge centroid toward best, an idea ...

	std::vector< std::vector<double> > simplex;	// store simplex as a vector of vectors [nvrt][ndof]
	std::vector<double> vrt_val (nvrt, 0.0);	// objective function values at vertices

	std::vector<double> centroid (ndof, 0.0);	// position of the opposite face centroid
	std::vector<double> del_vect (ndof, 0.0);	// vector from wrst to centroid

	std::vector<double> xref (ndof, 0.0);	// reflection
	std::vector<double> xexp (ndof, 0.0);	// expansion
	std::vector<double> xcon (ndof, 0.0);	// contraction

	double yref, yexp, ycon;

	simplex.resize(nvrt);
	for (int i=0; i<nvrt; i++)
	{
		simplex[i].resize(ndof);
	}

	for (int i=0; i<nvrt; i++)
	{
		for (int j=0; j<ndof; j++)
		{
			simplex[i][j] = start[j];
		}

		if (i != 0)
		{
			simplex[i][i-1] += dels[i-1];
		}
	}

//	std::cout << "\n";				// output initial simplex vertices
//	for (int i=0; i<nvrt; i++) {
//		for (int j=0; j<ndof; j++)
//			std::cout << simplex[i][j] << "\t";
//		std::cout << "\n";
//	}

	for (int i=0; i<nvrt; i++)
	{			// get vertex objective function values
		try
		{
			vrt_val[i] = obj_val_at(simplex[i]);
		}
		catch (Range_Fail)
		{
			throw Range_Fail();
		}
	}

	num_func += nvrt;

// Note: standard formulation uses centroid as a reference point for new locations
// Note: there are two cases where the reflected point is kept: 1.a. and 2.b.
//
// 1. xref = reflect wrst beyond centroid by a factor of alpha (typically = 1; >0)
// 	a. if xref is between best and poor (second wrst), substitute xref for wrst and iterate
//
// 2. xexp = expand wrst beyond centroid by a factor of gamma (typically = 2; >alpha)
// 	a. if xexp is better than xref, substitute xexp for wrst and iterate
//	b. if xepx is not better than xref (but better than wrst), substitute xref for wrst and iterate
//
// 3. xcon = contract wrst within centroid by a factor of delta (typically 0.5; >0, <=0.5)
//	a. if xcon is better than wrst, substitute xcon for wrst and iterate
// 	b. if xcon not better than wrst, contract all but best toward best by a factor of omega (typically 0.5; >0, <=0.5)
//
	for (;;) {
		int wrst = 0;		// wrst, poor, best: poor = "second worst"
		int best = 0;
		for (int i=1; i<nvrt; i++)
		{
			if (vrt_val[i] > vrt_val[wrst])
			{
				wrst = i;
			}
			if (vrt_val[i] < vrt_val[best])
			{
				best = i;
			}
		}

		int poor = best;

		for (int i=0; i<nvrt; i++)
		{
			if ((i != wrst) && (i != best))
			{
				if (vrt_val[i] > vrt_val[poor])
				{
					poor = i;
				}
			}
		}
		// Note: the individual vertex values are updated during simplex evaluation below

		double rel_diff = 2.0*fabs(vrt_val[wrst]-vrt_val[best])/(fabs(vrt_val[wrst])+fabs(vrt_val[best])+NOT_ZERO);

		if (rel_diff < conv_tol)
		{
			return simplex[best];	// successful convergence, normal return from minimize
		}

		if (num_func > MAX_FUNC)
		{
			throw Convg_Fail();	// wandering around aimlessly
		}

		// prepare for simplex modification

		for (int j=0; j<ndof; j++)
		{
			centroid[j] = 0.0;
			for (int i=0; i<nvrt; i++)
			{
				if (i != wrst)
				{
					centroid[j] += simplex[i][j];
				}
			}
			centroid[j] /= ndof;
			del_vect[j] = centroid[j] - simplex[wrst][j];
//			del_vect[j] = centroid[j] - simplex[wrst][j] + NUDGE*(simplex[best][j] - centroid[j]);
		}

		bool eval = true;
		while (eval) {		// evaluate and adjust the simplex

			for (int i=0; i<ndof; i++)
			{
				xref[i] = centroid[i] + ALPHA*del_vect[i];
			}
			try
			{
				yref = obj_val_at(xref);
			}
			catch (Range_Fail)
			{
				throw Range_Fail();
			}

			num_func += 1;

			if ((yref <= vrt_val[poor]) && (yref >= vrt_val[best]))
			{
				simplex[wrst] = xref;
				vrt_val[wrst] = yref;
				break;
			}

			if (yref < vrt_val[best])
			{
				for (int i=0; i<ndof; i++)
				{
					xexp[i] = centroid[i] + GAMMA*del_vect[i];
				}
				try
				{
					yexp = obj_val_at(xexp);
				}
				catch (Range_Fail)
				{
					throw Range_Fail();
				}

				num_func += 1;

				if (yexp < vrt_val[best])
				{
					simplex[wrst] = xexp;
					vrt_val[wrst] = yexp;
					break;
				}
				else
				{
					simplex[wrst] = xref;
					vrt_val[wrst] = yref;
					break;
				}
			}

			for (int i=0; i<ndof; i++)
			{
				xcon[i] = centroid[i] + DELTA*del_vect[i];
			}

			try
			{
				ycon = obj_val_at(xcon);
			}
			catch (Range_Fail)
			{
				throw Range_Fail();
			}

			num_func += 1;

			if (ycon < vrt_val[best])
			{
				simplex[wrst] = xcon;
				vrt_val[wrst] = ycon;
				break;
			}
			else
			{
				for (int i=0; i<nvrt; i++)
				{
					if (i != best)
					{
						for (int j=0; j<ndof; j++)
						{
							simplex[i][j] = simplex[best][j] + OMEGA*(simplex[i][j] - simplex[best][j]);
						}
						try
						{
							vrt_val[i] = obj_val_at(simplex[i]);
						}
						catch (Range_Fail)
						{
							throw Range_Fail();
						}

						num_func += 1;
					}
				}
				break;
			}

			eval = false;	// code should never get here, but break loop in case
					// simplex has not changed, for will run until MAX_FUNC reached

		} // while end


	} // for end



}
/******************************************************************************/
void Search::trgrid_global(double displ_max, double basin_radius, int n)
{
	// use basin_fraction as a basis for controlloing the resolution of the global search
	// 0.0 < basin_fraction <= 1.0, check on input
	// full float version, very slow with tricubic, much faster with trilinear

	// check for opt out of coarse search
	if (basin_radius == 0.0) return;

	int num_inc = displ_max/basin_radius;
	double inc = displ_max/num_inc;
	int num_pts = (2*num_inc+1)*(2*num_inc+1)*(2*num_inc+1);

	// objective function mapping
	// std::vector<double> obj_vals(num_pts,0.0);
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
		// obj_vals[count] = obj_val;
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

	return;

	// objective function mapping, need basin_radius on
	// optional output block, write as a raw 8-bit image volume
/*
	if (n == 0)
	std::cout << "\n\n mapping start point objective function \n\n";
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
*/
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

}/******************************************************************************/
/*Search::Min_Ftor_new::Min_Ftor_new(const Interp_Type int_typ, const Objfcn_Type obj_typ, const int ndof, FloatingCloud *fcld, Interpolate *interp, const std::vector<double> &ref_subvol)
// not currently used, but leave in place for now
{

	lndof = ndof;
	lfcld = fcld;
	linterp = interp;

	lpar_cur = std::vector<double>(12,0.0);
	lref_subvol = std::vector<double>(ref_subvol.size(),0.0);
	ltar_subvol = std::vector<double>(ref_subvol.size(),0.0);

	lref_subvol = ref_subvol;

	lobj_typ = obj_typ;
	lint_typ = int_typ;
}

double Search::Min_Ftor_new::operator() (const std::vector<double> x)
{
	for (int i=0; i<x.size(); i++)
		lpar_cur[i] = x[i];


	lfcld->affine_to(lpar_cur, lndof);

	if (lint_typ == trilinear)
	{
		try {linterp->tri_lin(lfcld->moving->ptvect, lfcld->moving->bbox(), ltar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}
	}

	if (lint_typ == tricubic)
	{
		try {linterp->tri_cub_Lek(lfcld->moving->ptvect, lfcld->moving->bbox(), ltar_subvol);}
		catch (Intrp_Fail) {throw Range_Fail();}
	}

	double obj;

	if (lobj_typ == SAD) obj = obj_SAD(lref_subvol, ltar_subvol);
	if (lobj_typ == SSD) obj = obj_SSD(lref_subvol, ltar_subvol);
	if (lobj_typ == ZSSD) obj = obj_ZSSD(lref_subvol, ltar_subvol);
	if (lobj_typ == NSSD) obj = obj_NSSD(lref_subvol, ltar_subvol);
	if (lobj_typ == ZNSSD) obj = obj_ZNSSD(lref_subvol, ltar_subvol);

	return obj;
}*/
/******************************************************************************/

#if defined(_WIN32) || defined(__WIN32__)
/**/
#else
std::ostream& operator<<(std::ostream &strm, const Search &a) {
	RunControl * run = a.rc;
	std::string objfun;
	if (run->obj_fcn == SAD) {
		//obj_fcn = &obj_SAD;
		objfun = std::string("objective function SAD");
	}
	if (run->obj_fcn == SSD) {
		//obj_fcn = &obj_SSD;
		objfun = std::string("objective function SSD");
	}
	if (run->obj_fcn == ZSSD) {
		//obj_fcn = &obj_ZSSD;
		objfun = std::string("objective function ZSSD");
	}
	if (run->obj_fcn == NSSD) {
		//obj_fcn = &obj_NSSD;
		objfun = std::string("objective function NSSD");
	}
	if (run->obj_fcn == ZNSSD) {
		//obj_fcn = &obj_ZNSSD;
		objfun = std::string("objective function ZNSSD");
	}

	return strm << "Search(" << std::endl <<
		"bytes_per " << a.bytes_per << std::endl <<
		"subv_rad " << a.subv_rad << std::endl <<
		"subv_num " << a.subv_num << std::endl <<
		"obj_fun " << objfun << std::endl <<
		"input shape " << run->vol_wide << " " <<
		run->vol_high << " " <<
		run->vol_tall << std::endl <<
		"subvol aspect " << run->subvol_aspect[0] << " " <<
		run->subvol_aspect[1] << " " <<
		run->subvol_aspect[2] << std::endl <<
		"numr_search_dof " << run->num_srch_dof << std::endl <<
		"disp max " << run->disp_max << std::endl <<
		")";
}
#endif
