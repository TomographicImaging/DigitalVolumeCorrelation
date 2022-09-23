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
           Shrikanth Nagella,
		   
*/


/* Storage of a 3D point. The point itself is stored as a double, with (x,y,z)
coordinates available through (.x,.y,.z) member functions. The point is also
split by the constructor and stored as an integer portion (.ix,.iy,.iz) and
a remainder portion (.rx,.ry,.rz). This facilitates interaction with transform
functions (which operate on the intact double) and interpolation functions
(which identify interpolation region through the integer portion, and location
within the region through the remainder portion). The split allows the use of
(.ix,.iy,.iz) components directly as array indices. */

#ifndef POINT_H
#define POINT_H

#include <cmath>

#include "CCPiDefines.h"
/*****************************************************************************/
class CCPI_EXPORT Point
{
public:
	Point(double xx, double yy, double zz);

	void move_to(double xx, double yy, double zz);
	void move_by(double u, double v, double w);
	
	inline double pt_dist(Point to) {
	return sqrt(pow(x()-to.x(),2) + pow(y()-to.y(),2) + pow(z()-to.z(),2));}
	
	inline double pt_encl_vol(Point to) { // in vox^3
	return ((fabs(x()-to.x())+1)*(fabs(y()-to.y())+1)*(fabs(z()-to.z())+1));}

	double x() const {return dx;}
	double y() const {return dy;}
	double z() const {return dz;}

	int ix() const {return ipx;}
	int iy() const {return ipy;}
	int iz() const {return ipz;}

	double rx() const {return rpx;}
	double ry() const {return rpy;}
	double rz() const {return rpz;}

private:
	double dx, dy, dz;
	int ipx, ipy, ipz;
	double rpx, rpy, rpz;

	void set_point(double xx, double yy, double zz);
	void parse_point();
};
/******************************************************************************/

#endif


