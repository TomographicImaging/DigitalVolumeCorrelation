
/**
# -*- coding: utf-8 -*-
#   This work is part of the Core Imaging Library developed by
#   Visual Analytics and Imaging System Group of the Science Technology
#   Facilities Council, STFC

#   Copyright 2018 CCPi

#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at

#       http://www.apache.org/licenses/LICENSE-2.0

#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

#   Code is derived from code developed by Prof. Brian Bay
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
//	DataCloud(InputRead *in);

	// sort cloud to establish point run order and neighbors (for starting points and strain calc)
	// needs points and labels already available, generates order and neigh
	void sort_order_neighbors(int neigh_num_save);

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
						// [nnbr] may vary by point
	
	std::vector< std::vector<ResultRecord> > results;	// [nres][npts]
	
	
	
	
	// trial idea ... create subgroups within order based on resorting
	// sub_groups contain indices of points in the main list
	// could subdivide points into fixed-number groups, with last one smaller
	// could subdivide in groups of fixed BoundBox size for interpolator use
	std::vector< std::vector<int> > sub_groups;	// [ngrp][nppg]
							// [nppg] may vary
	std::vector<Cloud> sub_clouds;	// probably better
	// volume in vox^3
	void split_by_volume(std::vector<DualSort> indx_dist, double vol_limit);
	// end trial
	
	
	
	int n_n_m() const {return neigh_num_min;}
//	int n_n_p() const {return neigh_num_par;}
	double n_d_p() const {return neigh_dst_par;}


private:
	
	int start_point_label;		// label of first point to process
	int start_point_index;		// index corresponding to label
	
	int neigh_num;			// num of neighbors to save for each point


	int neigh_num_min;		// minimum number for strain calc
//	int neigh_num_par;		// number of neighbors parameter
	double neigh_dst_par;		// distance range parameter

};
/******************************************************************************/

#endif
