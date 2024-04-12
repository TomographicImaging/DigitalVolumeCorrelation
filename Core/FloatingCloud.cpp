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
*/
#include "FloatingCloud.h"

/******************************************************************************/
FloatingCloud::FloatingCloud (double ref_x, double ref_y, double ref_z)
{
	stable = new Cloud();
	moving = new Cloud();

	params = new SearchParams();

	stable_refpt = new Point(ref_x, ref_y, ref_z);
	moving_refpt = new Point(ref_x, ref_y, ref_z);
}
/*
public:
  Foo(char x, int y) {}
  Foo(int y) : Foo('a', y) {} // Foo(int) delegates to Foo(char,int)
};
*/
/******************************************************************************/
FloatingCloud::FloatingCloud (Point cen, double rad_max, int num, double aspect_x, double aspect_y, double aspect_z, unsigned int seed)
{
	stable = new Cloud();
	moving = new Cloud();

	params = new SearchParams();

	stable_refpt = new Point(cen.x(), cen.y(), cen.z());
	moving_refpt = new Point(cen.x(), cen.y(), cen.z());

	//using random.h now
	std::mt19937 mt(seed);

	std::uniform_real_distribution<double> dist(-rad_max, rad_max);

// subvolo echo
// std::ofstream svol_file("subvol_points.txt", std::ios_base::app);	// subvol echo
//

	int point_count = 0;
	while (point_count < num) {

		double rel_x = dist(mt);
		double rel_y = dist(mt);
		double rel_z = dist(mt);

		double radius = sqrt(rel_x*rel_x + rel_y*rel_y + rel_z*rel_z);

		if (radius <= rad_max) {
			point_count += 1;

			double x = cen.x() + aspect_x*(rel_x);
			double y = cen.y() + aspect_y*(rel_y);
			double z = cen.z() + aspect_z*(rel_z);

			Point apt = Point(x, y, z);
			AddPoint(apt);
			//if (float_cloud_num == 0) svol_file << x << "\t" << y << "\t" << z << "\n";	// subvol echo
		}
	 }

// float_cloud_num += 1;							// subvol echo
}
/******************************************************************************/
FloatingCloud::FloatingCloud (Point box_min, Point box_max, int nx, int ny, int nz)
// cube
{
	// check that box_min and box_max are OK

	stable = new Cloud();
	moving = new Cloud();

	params = new SearchParams();

	double avg_x = (box_max.x() + box_min.x())/2.0;
	double avg_y = (box_max.y() + box_min.y())/2.0;
	double avg_z = (box_max.z() + box_min.z())/2.0;

	stable_refpt = new Point(avg_x, avg_y, avg_z);
	moving_refpt = new Point(avg_x, avg_y, avg_z);

	double inc_x = (box_max.x() - box_min.x())/(nx-1);
	double inc_y = (box_max.y() - box_min.y())/(ny-1);
	double inc_z = (box_max.z() - box_min.z())/(nz-1);

	double pos_x = box_min.x();
	double pos_y = box_min.y();
	double pos_z = box_min.z();

	// subvol echo
	// std::ofstream svol_file("subvol_points.txt", std::ios_base::app);
	//

	for (int i=0; i<nz; i++) {
		pos_z = box_min.z() + i*inc_z;
		for (int j=0; j<ny; j++) {
			pos_y = box_min.y() + j*inc_y;
			for (int k=0; k<nx; k++) {
				pos_x = box_min.x() + k*inc_x;
				Point apt = Point(pos_x, pos_y, pos_z);
				AddPoint(apt);
	// subvol echo
	// svol_file << pos_x << "\t" << pos_y << "\t" << pos_z << "\n";	// subvol echo
	//
			}
		}
	}
// float_cloud_num += 1;							// subvol echo
}
/****b**************************************************************************/
FloatingCloud::~FloatingCloud()
{
	delete stable;
	delete moving;

	delete params;

	delete stable_refpt;
	delete moving_refpt;
}
/******************************************************************************/
int FloatingCloud::AddPoint(Point new_point)
{
	stable->AddPoint(new_point);
	moving->AddPoint(new_point);
	return 1;
}
/******************************************************************************/
void FloatingCloud::strain_ptvect(std::vector<Point> &ptvect)
{
	double S00 = params->tens_str()[0][0];
	double S01 = params->tens_str()[0][1];
	double S02 = params->tens_str()[0][2];
	double S10 = params->tens_str()[1][0];
	double S11 = params->tens_str()[1][1];
	double S12 = params->tens_str()[1][2];
	double S20 = params->tens_str()[2][0];
	double S21 = params->tens_str()[2][1];
	double S22 = params->tens_str()[2][2];

	for (unsigned int i=0; i<ptvect.size(); i++) {
		// rel is a position vector wrt the cloud reference point
		double rel_x = ptvect[i].x() - moving_refpt->x();
		double rel_y = ptvect[i].y() - moving_refpt->y();
		double rel_z = ptvect[i].z() - moving_refpt->z();

		double str_x =	S00*rel_x + S01*rel_y + S02*rel_z;
		double str_y =	S10*rel_x + S11*rel_y + S12*rel_z;
		double str_z =	S20*rel_x + S21*rel_y + S22*rel_z;

		ptvect[i].Point::move_by(str_x, str_y, str_z);
	}
}
/******************************************************************************/
void FloatingCloud::rotate_ptvect(std::vector<Point> &ptvect)
{
	double A00 = this->params->matr_rot()[0][0];
	double A01 = this->params->matr_rot()[0][1];
	double A02 = this->params->matr_rot()[0][2];
	double A10 = this->params->matr_rot()[1][0];
	double A11 = this->params->matr_rot()[1][1];
	double A12 = this->params->matr_rot()[1][2];
	double A20 = this->params->matr_rot()[2][0];
	double A21 = this->params->matr_rot()[2][1];
	double A22 = this->params->matr_rot()[2][2];

	for (unsigned int i=0; i<ptvect.size(); i++) {
		// rel is a position vector wrt the cloud reference point
		double rel_x = ptvect[i].x() - this->moving_refpt->x();
		double rel_y = ptvect[i].y() - this->moving_refpt->y();
		double rel_z = ptvect[i].z() - this->moving_refpt->z();

		// apply the rotation matrix to the rel vector
		double rot_x =	A00 * rel_x + A01 * rel_y + A02 * rel_z;
		double rot_y =	A10 * rel_x + A11 * rel_y + A12 * rel_z;
		double rot_z =	A20 * rel_x + A21 * rel_y + A22 * rel_z;

		// delta between the rot and rel vectors
		double del_x = rot_x - rel_x;
		double del_y = rot_y - rel_y;
		double del_z = rot_z - rel_z;

		ptvect[i].Point::move_by(del_x, del_y, del_z);
	}
}
/******************************************************************************/
void FloatingCloud::resize_bbox(std::vector<Point> &ptvect)
{
	double min_x = 9999.9;	// adjust bounding box
	double min_y = 9999.9;
	double min_z = 9999.9;

	double max_x = -9999.9;
	double max_y = -9999.9;
	double max_z = -9999.9;

	bool reset = false;

	for (unsigned int i=0; i<ptvect.size(); i++) {
		if (ptvect[i].x() < min_x) {min_x = ptvect[i].x(); reset = true;}
		if (ptvect[i].y() < min_y) {min_y = ptvect[i].y(); reset = true;}
		if (ptvect[i].z() < min_z) {min_z = ptvect[i].z(); reset = true;}

		if (ptvect[i].x() > max_x) {max_x = ptvect[i].x(); reset = true;}
		if (ptvect[i].y() > max_y) {max_y = ptvect[i].y(); reset = true;}
		if (ptvect[i].z() > max_z) {max_z = ptvect[i].z(); reset = true;}
	}

	if (reset) {
		Point minpt = Point(min_x, min_y, min_z);
		Point maxpt = Point(max_x, max_y, max_z);
		moving->Cloud::reset_box(minpt,maxpt);
	}
}
/******************************************************************************/

void FloatingCloud::affine_to (const std::vector<double> &t_vect, int ndof)
/* A key routine, and somewhat complex. The degrees of freedom are grouped as
	 (translation), (translation, rotation), (translation, rotation, strain).
	 For any transform, the floating bbox is adjusted to encompas the cloud.
*/
{
	// fill the complete param vector, and form required matrices
	params->set_param_vect(t_vect, ndof);

	// translation transform
	double del_x = params->vect_all()[0];
	double del_y = params->vect_all()[1];
	double del_z = params->vect_all()[2];

	 // first move the floating cloud reference point
	double new_ref_x = stable_refpt->x() + del_x;
	double new_ref_y = stable_refpt->y() + del_y;
	double new_ref_z = stable_refpt->z() + del_z;
	moving_refpt->Point::move_to(new_ref_x, new_ref_y, new_ref_z);

	// now move the floating cloud bounding box
	double new_min_x = stable->bbox()->min().x() + del_x;
	double new_min_y = stable->bbox()->min().y() + del_y;
	double new_min_z = stable->bbox()->min().z() + del_z;

	double new_max_x = stable->bbox()->max().x() + del_x;
	double new_max_y = stable->bbox()->max().y() + del_y;
	double new_max_z = stable->bbox()->max().z() + del_z;

	Point minpt = Point(new_min_x, new_min_y, new_min_z);
	Point maxpt = Point(new_max_x, new_max_y, new_max_z);
	moving->Cloud::reset_box(minpt,maxpt);

	// now move all the individual points
	for (unsigned int i=0; i<moving->ptvect.size(); i++) {
		double new_x = stable->ptvect[i].x() + del_x;
		double new_y = stable->ptvect[i].y() + del_y;
		double new_z = stable->ptvect[i].z() + del_z;

		moving->ptvect[i].Point::move_to(new_x, new_y, new_z);
	}

	// additional transforms beyond simple translation
	if (ndof > 3) {
		rotate_ptvect(moving->ptvect);
		if (ndof > 6)
			strain_ptvect(moving->ptvect);
		resize_bbox(moving->ptvect);
	}

}
/******************************************************************************/
void FloatingCloud::echo_stable_ref()
{
	std::cout << std::endl;
	std::cout << stable_refpt->x() << std::endl;
	std::cout << stable_refpt->y() << std::endl;
	std::cout << stable_refpt->z() << std::endl;
	std::cout << std::endl;
}
/******************************************************************************/
void FloatingCloud::echo_moving_ref()
{
	std::cout << std::endl;
	std::cout << moving_refpt->x() << std::endl;
	std::cout << moving_refpt->y() << std::endl;
	std::cout << moving_refpt->z() << std::endl;
	std::cout << std::endl;
}
/******************************************************************************/
void FloatingCloud::file_cloud_data(std::string ofname)
{
	std::ofstream ofs;
	ofs.open(ofname.c_str(), std::ofstream::out);
	if (!ofs){std::cout <<"\n-> Can't open " << ofname << "\n\n";return;}

	for (unsigned int i=0; i<moving->ptvect.size(); i++) {
		double stab_x = stable->ptvect[i].x();
		double stab_y = stable->ptvect[i].y();
		double stab_z = stable->ptvect[i].z();

		double actv_x = moving->ptvect[i].x();
		double actv_y = moving->ptvect[i].y();
		double actv_z = moving->ptvect[i].z();

		double disp_x = actv_x - stab_x;
		double disp_y = actv_y - stab_y;
		double disp_z = actv_z - stab_z;

		double rela_x = actv_x - (moving_refpt->x() - stable_refpt->x()) - stab_x;
		double rela_y = actv_y - (moving_refpt->y() - stable_refpt->y()) - stab_y;
		double rela_z = actv_z - (moving_refpt->z() - stable_refpt->z()) - stab_z;

		ofs << stab_x << "\t" << stab_y << "\t" << stab_z << "\t";
		ofs << actv_x << "\t" << actv_y << "\t" << actv_z << "\t";
		ofs << disp_x << "\t" << disp_y << "\t" << disp_z << "\t";
		ofs << rela_x << "\t" << rela_y << "\t" << rela_z << "\t";

		if (i==0) {
			ofs << moving->bbox()->min().x() << "\t";
			ofs << moving->bbox()->max().x() << "\t";
			ofs << moving->bbox()->max().x() << "\t";
			ofs << moving->bbox()->min().x() << "\t";
			ofs << moving->bbox()->min().x() << "\t";
			ofs << moving->bbox()->max().x() << "\t";
			ofs << moving->bbox()->max().x() << "\t";
			ofs << moving->bbox()->min().x() << "\t";
		}

		if (i==1) {
			ofs << moving->bbox()->min().y() << "\t";
			ofs << moving->bbox()->min().y() << "\t";
			ofs << moving->bbox()->max().y() << "\t";
			ofs << moving->bbox()->max().y() << "\t";
			ofs << moving->bbox()->min().y() << "\t";
			ofs << moving->bbox()->min().y() << "\t";
			ofs << moving->bbox()->max().y() << "\t";
			ofs << moving->bbox()->max().y() << "\t";
		}

		if (i==2) {
			ofs << moving->bbox()->min().z() << "\t";
			ofs << moving->bbox()->min().z() << "\t";
			ofs << moving->bbox()->min().z() << "\t";
			ofs << moving->bbox()->min().z() << "\t";
			ofs << moving->bbox()->max().z() << "\t";
			ofs << moving->bbox()->max().z() << "\t";
			ofs << moving->bbox()->max().z() << "\t";
			ofs << moving->bbox()->max().z() << "\t";
		}
		ofs << "\n";
	};

	ofs.close();
}
/******************************************************************************/
