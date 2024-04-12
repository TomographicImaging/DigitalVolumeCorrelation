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
           Gemma Fardell (UKRI-STFC)
*/
#ifndef FLOATINGCLOUD_H
#define FLOATINGCLOUD_H

#include <vector>
#include <string>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <iostream>
#include <time.h>
#include <random>
#include <chrono>

// adjust Makefile if changes here
#include "Point.h"
#include "Cloud.h"
#include "SearchParams.h"
//
#include "CCPiDefines.h"
/******************************************************************************/
class CCPI_EXPORT FloatingCloud {
public:
	/* Create an empty FloatingCloud with a reference point specified. In general,
	points would be read from a file for this constructor */
	FloatingCloud(double ref_x, double ref_y, double ref_z);
	
	/* Create num random points in a sphere of radius rad at center. */
	FloatingCloud(Point cen, double rad, int num, double aspect_x, double aspect_y, double aspect_z, unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count());

	/* Create with (nx,ny,nz) points spanning box_min..box_max. */
	FloatingCloud(Point box_min, Point box_max, int nx, int ny, int nz);
	
	~FloatingCloud();
	
	Cloud *stable;
	Cloud *moving;
	
	SearchParams *params;
	
	Point *stable_refpt;
	Point *moving_refpt;

	/* Use FloatingCloud::AddPoint to make sure both stable and float clouds are
	 initially populated with the same points */
	int AddPoint(Point new_point);
	
	/* Calls SearchParams::set_param_vect_all to repopulate the params to the
	 input value, then applies the transform to the stable cloud, producing a new
	 floating cloud. The degrees of freedom are grouped as (translation x 3,
	 rotation x 3, strain x 3). For any transform, the floating bbox is adjusted
	 to encompass the cloud. */
	void affine_to(const std::vector<double> &t_vect, int ndof);
	
	void echo_stable_ref();
	void echo_moving_ref();
	
	/* Creates a tab'd output file of cloud data for checking. */
	void file_cloud_data(std::string ofname);

private:
	void strain_ptvect(std::vector<Point> &ptvect);
	void rotate_ptvect(std::vector<Point> &ptvect);
	void resize_bbox(std::vector<Point> &ptvect);
};
/******************************************************************************/

#endif
