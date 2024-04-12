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

Author(s): Brian Bay (OSU),
           Shrikanth Nagella (UKRI-STFC),
           Edoardo Pasca (UKRI-STFC),
           Gemma Fardell (UKRI-STFC)
*/
#include "BoundBox.h"

/******************************************************************************/
BoundBox::~BoundBox()
{
	delete box_min;
	delete box_max;
	delete box_cen;
}
/******************************************************************************/
BoundBox::BoundBox(Point bb_min, Point bb_max)
{
	// add checks on point validity here

	double x_avg = (bb_min.x() + bb_max.x())/2.0;
	double y_avg = (bb_min.y() + bb_max.y())/2.0;
	double z_avg = (bb_min.z() + bb_max.z())/2.0;

	box_min =  new Point(bb_min.x(), bb_min.y(), bb_min.z());
	box_max =  new Point(bb_max.x(), bb_max.y(), bb_max.z());
	box_cen =  new Point(x_avg, y_avg, z_avg);
}
/******************************************************************************/
void BoundBox::set_center()
{
	double x_avg = (box_min->x() + box_max->x())/2.0;
	double y_avg = (box_min->y() + box_max->y())/2.0;
	double z_avg = (box_min->z() + box_max->z())/2.0;

	box_cen->Point::move_to(x_avg, y_avg, z_avg);
}
/******************************************************************************/
void BoundBox::move_to(Point min_pt, Point max_pt)
{
	// add checks on point validity here

	box_min->Point::move_to(min_pt.x(), min_pt.y(), min_pt.z());
	box_max->Point::move_to(max_pt.x(), max_pt.y(), max_pt.z());

	set_center();
}
/******************************************************************************/
void BoundBox::grow_by(double frame)
{
	// add checks on point validity here

	box_min->Point::move_by(-frame, -frame, -frame);
	box_max->Point::move_by( frame,  frame,  frame);

	set_center();
}
/******************************************************************************/
void BoundBox::center_on(Point center)
// shift box to center
{
	double x_del = center.x() - box_cen->x();
	double y_del = center.y() - box_cen->y();
	double z_del = center.z() - box_cen->z();

	box_min->Point::move_by(x_del, y_del, z_del);
	box_max->Point::move_by(x_del, y_del, z_del);
	box_cen->Point::move_by(x_del, y_del, z_del);
}
/******************************************************************************/
void BoundBox::contains(const BoundBox *bbox2)
// Checks whether bbox2 is within the instantiated box.
{
	if (bbox2->min().x() < box_min->x() ||
	    bbox2->min().y() < box_min->y() ||
	    bbox2->min().z() < box_min->z() ||

	    bbox2->max().x() > box_max->x() ||
	    bbox2->max().y() > box_max->y() ||
	    bbox2->max().z() > box_max->z() )

	throw Bound_Fail();
}
/******************************************************************************/

