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
           Edoardo Pasca (UKRI-STFC)
*/

/* Cloud contains the most basic information and functions associated with
a collection of Point data. It starts empty, with points added through
AddPoint, which updates box to reflect the change. */

#ifndef CLOUD_H
#define CLOUD_H

#include <iostream>
#include <vector>

// adjust Makefile if changes made here
#include "Point.h"
#include "BoundBox.h"

#include "CCPiDefines.h"
//

/******************************************************************************/
class CCPI_EXPORT Cloud
{
public:
	Cloud();
	~Cloud();

	std::vector<Point> ptvect;

	BoundBox *bbox() const {return box;}

	void AddPoint(Point new_point);

	void reset_box(Point minpt, Point maxpt);

	void echo_summary();
	void echo_content();

private:

	BoundBox *box;	// fixed for stable and data clouds, updated for moving
	
};
/******************************************************************************/

#endif
