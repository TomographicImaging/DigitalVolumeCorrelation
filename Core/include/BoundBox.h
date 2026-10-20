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
           Srikanth Nagella (UKRI-STFC)
		   
*/
/* A simple class that stores lower and upper corners of a box.
Simplifies including in classes and passing to other functions.
Implement checking that the corner designations are legitimate. */

#ifndef BOUNDBOX_H
#define BOUNDBOX_H

// adjust Makefile if changes made here
#include "Point.h"
#include "Utility.h"
//

/******************************************************************************/
class BoundBox
{
public:
	BoundBox(Point bb_min, Point bb_max);
	~BoundBox();

	void set_center();

	void move_to(Point min_pt, Point max_pt);
	void grow_by(double frame);
	void center_on(Point center);

	void contains(const BoundBox *bbox2);

	Point min() const {return *box_min;}
	Point max() const {return *box_max;}
	Point cen() const {return *box_cen;}

	int iwide() const {return box_max->ix() - box_min->ix();}
	int ihigh() const {return box_max->iy() - box_min->iy();}
	int itall() const {return box_max->iz() - box_min->iz();}
	
	inline double box_vol() {
	return (box_max->x()-box_min->x()+1)*(box_max->y()-box_min->y()+1)*(box_max->z()-box_min->z()+1);}

private:

	Point *box_min;
	Point *box_max;
	Point *box_cen;
};
/******************************************************************************/

#endif
