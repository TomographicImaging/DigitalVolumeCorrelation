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
           
*/
#include "Point.h"

/******************************************************************************/
Point::Point(double xx, double yy, double zz)

/* Standard constructor stores the intact point location in private member
variables, and also splits the components into integer and remainder parts. */

{
	set_point(xx,yy,zz);

	parse_point();
}
/******************************************************************************/
void Point::move_to(double xx, double yy, double zz)
{
	set_point(xx,yy,zz);

	parse_point();
}
/******************************************************************************/
void Point::move_by(double u, double v, double w)
{
	set_point(dx + u, dy + v, dz + w);

	parse_point();
}
/******************************************************************************/
void Point::set_point(double xx, double yy, double zz)
{
	dx = xx;
	dy = yy;
	dz = zz;
}
/******************************************************************************/
//void Point::parse_point(double xx, double yy, double zz)
void Point::parse_point()
{
	ipx = dx/1;
	ipy = dy/1;
	ipz = dz/1;

	rpx = dx - ipx;
	rpy = dy - ipy;
	rpz = dz - ipz;
}
/******************************************************************************/


