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
