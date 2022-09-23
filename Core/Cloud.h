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
		   Edoardo Pasca,
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
