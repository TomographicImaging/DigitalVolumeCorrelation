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
		   Gemma Fardell
*/
#include "Cloud.h"

/******************************************************************************/
/* Basic cloud constructor. The Cloud box is initialized to out-of-range values.
These are updated as points are added to the Cloud via AddPoint. This
constructor takes no arguments, and is used when no reference point exists. */

Cloud::Cloud()
{
	Point box_min_limit(9999.99, 9999.99, 9999.99);
	Point box_max_limit(-9999.99, -9999.99, -9999.99);

	box = new BoundBox(box_min_limit, box_max_limit);
}

/*****************************************************************************/
Cloud::~Cloud()
{
	delete box;
}
/******************************************************************************/
void Cloud::AddPoint(Point new_point)
{	
	ptvect.push_back(new_point);	// add a point to the list

	double min_x = box->min().x();	// adjust bounding box
	double min_y = box->min().y();
	double min_z = box->min().z();

	double max_x = box->max().x();
	double max_y = box->max().y();
	double max_z = box->max().z();

	bool reset = false;

	if (new_point.x() < min_x) {min_x = new_point.x(); reset = true;}
	if (new_point.y() < min_y) {min_y = new_point.y(); reset = true;}
	if (new_point.z() < min_z) {min_z = new_point.z(); reset = true;}

	if (new_point.x() > max_x) {max_x = new_point.x(); reset = true;}
	if (new_point.y() > max_y) {max_y = new_point.y(); reset = true;}
	if (new_point.z() > max_z) {max_z = new_point.z(); reset = true;}

	if (reset) {
		Point minpt(min_x, min_y, min_z);
		Point maxpt(max_x, max_y, max_z);
		box->BoundBox::move_to(minpt, maxpt);
	}
}
/******************************************************************************/
void Cloud::reset_box(Point minpt, Point maxpt)
{
	box->BoundBox::move_to(minpt, maxpt);
}
/******************************************************************************/
void Cloud::echo_summary()
{
	std::cout << "num pts = " << ptvect.size() << std::endl;

	std::cout << "box = ";
	std::cout << box->min().x() << " ";
	std::cout << box->min().y() << " ";
	std::cout << box->min().z() << std::endl;
	std::cout << box->max().x() << " ";
	std::cout << box->max().y() << " ";
	std::cout << box->max().z() << std::endl;
}
/******************************************************************************/
void Cloud::echo_content()
{
	for (unsigned int i=0; i<ptvect.size(); i++)			// echo
	{
		std::cout << ptvect[i].x() << "\t" << ptvect[i].y() << "\t" << ptvect[i].z() << std::endl;
		std::cout << ptvect[i].ix() << "\t" << ptvect[i].iy() << "\t" << ptvect[i].iz() << std::endl;
		std::cout << ptvect[i].rx() << "\t" << ptvect[i].ry() << "\t" << ptvect[i].rz() << std::endl;
	}
}
/******************************************************************************/

