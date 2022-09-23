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
