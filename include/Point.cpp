
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


