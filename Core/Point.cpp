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


