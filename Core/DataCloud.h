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
		   Edoardo Pasca,
*/
#ifndef DATACLOUD_H
#define DATACLOUD_H

#include <vector>
#include <algorithm>

// adjust Makefile if changes made here
#include "Point.h"
#include "Cloud.h"
#include "InputRead.h"
#include "Utility.h"
#include "CCPiDefines.h"
#include <omp.h>
#include <ostream>
//

/******************************************************************************/
struct DualSort
{
	int index;
	double value;
};
/******************************************************************************/
struct ResultRecord
// Information from a completed search
{
	int status;		// as defined in Utility.h
	
//	Objfcn_Type obj_typ;	// for interpretation of obj_min
	
	double obj_min;			// value at minumum
	
	std::vector<double> par_min;	// parameters at min [ndof]

};

/******************************************************************************/
class CCPI_EXPORT DataCloud
{
public:
	DataCloud();
	
	void organize_cloud(RunControl *run);

	// sort cloud to establish point run order and neighbors (for starting points and strain calc)
	// needs points and labels already available, generates order and neigh
	void sort_order_neighbors(Point starting_point);
	int nbr_num_save() const {return nbr_num_save_default;}

	// write out neighbors as a .sort file
	void write_sort_file(std::string fname, std::vector<std::vector<int>> &save_neigh);
	
	// in order of appearance in the point cloud input file, [npts]
	std::vector<Point> points;
	
	// in order of appearance in the point cloud input file, [npts]
	std::vector<int> labels;
	
	// indices of points in search order, eg start by distance from [npts]
	std::vector<int> order;
	
	// indices of neighbors of a search point
	std::vector< std::vector<int> > neigh;	// [npts][nnbr], includes self
	
	// vector of result records for a point
	std::vector< std::vector<ResultRecord> > results;	// [nres][npts]
	
	//
	// results from the STRAIN calculation executable
	//

	// Engineering strain components and principals
	std::vector< std::vector<double> > Estrain;	// exx,eyy,ezz,exy,eyz,exz,p1,p2,p3
	
	// Lagrangian strain components and principals
	std::vector< std::vector<double> > Lstrain;	// exx,eyy,ezz,exy,eyz,exz,p1,p2,p3	

	// displacement components computed at cloud locations from the volume fitting process
	std::vector< std::vector<double> > dis_vfit;	// u,v,w

	// strain window radius (without half the subvol size) computed at cloud locations
	std::vector<double> sw_rad;

	// for variable neighborhood approaches, num of pts in the strain window
	std::vector<int> pts_in_sw;

private:
	
//	int start_point_label;		// label of first point to process
	int start_point_index;		// index corresponding to label

	int nbr_num_save_default;	// default number used for .sort file, set in constructor
	
};
/******************************************************************************/

#endif
