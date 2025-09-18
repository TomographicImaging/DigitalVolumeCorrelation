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
           Edoardo Pasca (UKRI-STFC)
*/
/*
DataCloud will organize search point information: the location of the point, any
pre-search information (e.g. starting ponit estimates, results of previous
searches, etc.). Results of a current search (numerical and categorical) are
also stored. A complete dvc run will loop through all the points in the cloud.
*/

#include "DataCloud.h"

/******************************************************************************/

bool sortByValue(const DualSort &lhs, const DualSort &rhs) { return lhs.value < rhs.value; }
bool sortByIndex(const DualSort &lhs, const DualSort &rhs) { return lhs.index < rhs.index; }

/******************************************************************************/
DataCloud::DataCloud ()
{
	nbr_num_save_default = 75;		// store this many nearest neighbor points in the .sort file
	
}
/******************************************************************************/
void DataCloud::write_sort_file(std::string fname, std::vector<std::vector<int>> &save_neigh)
{
	std::cout << "Saving sorted pointcloud" << std::endl;

    std::ofstream sorted_pc_file;
	sorted_pc_file.open(fname + ".sort.csv");
	
/*
	for (auto &x : save_neigh) {
		for (auto &k : x)
			sorted_pc_file << k << ",";
		sorted_pc_file << std::endl;
	}
*/

	for (unsigned int i=0; i<save_neigh.size(); i++) {
		for (unsigned int j=0; j<save_neigh[i].size(); j++) {
			if (j>0) {sorted_pc_file << ",";}
			sorted_pc_file << save_neigh[i][j];
		}
		sorted_pc_file << std::endl;
	}

	sorted_pc_file.close();
}
/******************************************************************************/
void DataCloud::sort_order_neighbors(Point starting_point)
{
	int neigh_num_save = nbr_num_save() < points.size() ? nbr_num_save() : points.size();

//  this would reset the initial sort point based on a point label, might be used in future
//	start_point_label = 1;
//	for (int i=0; i<labels.size(); i++)
//		if (labels[i] == start_point_label) start_point_index = i;


	//start_point_index = 0;	// start with first point in the list, regardless of the label
	
	// *** estblish search order based on distance from start_point
	
	order.resize(points.size());

	std::vector<DualSort> indx_dist(points.size());
	for (int i=0; i<points.size(); i++) {
		indx_dist[i].index = i;
		indx_dist[i].value = starting_point.pt_dist(points[i]);
	}
	std::sort(indx_dist.begin(), indx_dist.end(), sortByValue);
	
	for (int i=0; i<points.size(); i++)
		order[i] = indx_dist[i].index;
	
	// *** get neighbors for each point	
	std::cout << "sorting point cloud" << std::endl;
	neigh.resize(points.size());
	std::vector<std::vector<int>> save_neigh = {};
	save_neigh.resize(points.size());
	
#pragma omp parallel
{
	int n_threads = omp_get_num_threads();
	std::vector<DualSort> indx_dist_copy(indx_dist);
	
# pragma omp for
	for (int i = 0; i < neigh.size(); i++) {

		for (int j = 0; j < neigh.size(); j++) {
			indx_dist_copy[j].index = j;
			indx_dist_copy[j].value = points[i].pt_dist(points[j]);
		}
		std::sort(indx_dist_copy.begin(), indx_dist_copy.end(), sortByValue);

		// this loads a set number
		for (int j = 0; j < neigh_num_save; j++)
			neigh[i].push_back(indx_dist_copy[j].index);
		
		
		for (int j = 0; j < neigh_num_save; j++)
			save_neigh[i].push_back(indx_dist_copy[j].index);
		

		// this loads a set number
		for (int j = 0; j < neigh_num_par; j++)
			neigh[i].push_back(indx_dist[j].index);
			
		
		
		for (int j = 0; j < neigh_num_save; j++)
			save_neigh[i].push_back(indx_dist[j].index);
		
		// indicate status for large point clouds
		int inc = 1000;
		if (n_threads == 1) {
			if ((neigh.size() > inc) && (i > inc - 1) && (i%inc == 0)) {
				std::cout << "sorting: " << i << " of " << neigh.size() << "\n";
			}
		}
		else {
			if (omp_get_thread_num() == 0) {
				int vi = n_threads * i;
				if ((neigh.size() > inc) && (vi > inc - 1) && (vi%inc == 0)) {
					std::cout << "sorting status: " << vi << " of " << neigh.size() << "\n";
				}
			}
		}
}
		
	}
	std::cout << "sorting finished" << std::endl;
   
}
/******************************************************************************/
void DataCloud::organize_cloud(RunControl *run)
{
	// logic to determine if a starting point is given or I should use the default
	Point starting_point = this->points[0];
	std::vector<double> nan_starting_point = { std::nan(""), std::nan(""), std::nan("")  };
	if (nan_starting_point != run->starting_point) {
		starting_point = Point(run->starting_point[0], run->starting_point[1], run->starting_point[2]);
	}
	// establish point processing order and neighborhoods
	sort_order_neighbors(starting_point);

	// write sort file
	write_sort_file(run->pts_fname, neigh);

	// this is where the integration with dvc executable occurs through results storage
	// *** create storage for results of searches and initialize
	
	int ntrg = 1;
	results.resize(ntrg);
	for (int i=0; i<ntrg; i++) {
		results[i].resize(points.size());
		for (int j=0; j<results[i].size(); j++) {
			results[i][j].status = not_search;
			results[i][j].obj_min = 0.0;
			results[i][j].par_min.resize(run->num_srch_dof, 0.0);
		}
	}

}
/******************************************************************************//******************************************************************************/
/*DataCloud::DataCloud (InputRead *in)
{
	// this is the old non parallel code

	// these are process control parameters set at run time
	// aim for a number of points within the neighborhood, let range adjust
	// aim for points within a range, let numbr adjust
	
	// neighbor number is difficult to manage
	// strain cal sets a lower limit of 6
	// easiest to manage with a set number
	// may also want to admit any points within a set distance
	// start point estimation requires just a few, even 1 valid point
	// small test clouds present a challenge if points are far apart
	
	neigh_num_min = 6;	// absolute minimum number for strain calc
	neigh_num_par = 8;	// just a test number for now is a guess for now
	neigh_dst_par = 15.0;	// a placeholder, scale to subvol size?
	
	if(!in->read_point_cloud(points, labels)) {throw Input_Fail();}
	
	// just a quick check to avoid problems with really small test clouds
	if (points.size() < neigh_num_par) neigh_num_par = points.size();
	
	start_point_label = 1;	// default 1, add as optional input parameter
//	start_point_label = 7;	// just a test ...
	start_point_index = 0;	// default 0, if not reset by label match
	
	for (int i=0; i<labels.size(); i++)
		if (labels[i] == start_point_label) start_point_index = i;
	

	// *** estblish search order based on distance from start_point
	
	order.resize(points.size());

	std::vector<DualSort> indx_dist(points.size());
	for (int i=0; i<points.size(); i++) {
		indx_dist[i].index = i;
		indx_dist[i].value = points[start_point_index].pt_dist(points[i]);
	}
	std::sort(indx_dist.begin(), indx_dist.end(), sortByValue);
	
	for (int i=0; i<points.size(); i++)
		order[i] = indx_dist[i].index;
	
	
	// *** break into groups based on re-sorted distance (testing)
	
//	split_by_volume(indx_dist, 2500.0);
//		
//	for (int i=0; i<sub_groups.size(); i++) {
//		for (int j=0; j<sub_groups[i].size(); j++)
//			std::cout << points[sub_groups[i][j]].x() << "\t" << points[sub_groups[i][j]].y() << "\t" << points[sub_groups[i][j]].z() << "\n";
//		std::cout << "\n\n";
//	}
	

	// *** get neighbors for each point	

	neigh.resize(points.size());
	
	for (int i=0; i<neigh.size(); i++) {
		for (int j=0; j<neigh.size(); j++) {
			indx_dist[j].index = j;
			indx_dist[j].value = points[i].pt_dist(points[j]);
		}
		std::sort(indx_dist.begin(), indx_dist.end(), sortByValue);
		
		// this loads a set number
		for (int j=0; j<neigh_num_par; j++)
			neigh[i].push_back(indx_dist[j].index);
		
		// this loads varying numbers of elements based on proximity
//		for (int j=0; j<neigh.size(); j++)
//			if (indx_dist[j].value <= neigh_dst_par) neigh[i].push_back(indx_dist[j].index);
	
	}
	
//	for (int i=0; i<neigh.size(); i++) {
//		for (int j=0; j<neigh[i].size(); j++)
//			std::cout << neigh[i][j] << "\t";
//		std::cout << "\n";
//	}

	
	
	// *** create storage for results of searches and initialize
	
	int ntrg = 1;
	results.resize(ntrg);
	for (int i=0; i<ntrg; i++) {
		results[i].resize(points.size());
		for (int j=0; j<results[i].size(); j++) {
			results[i][j].status = not_search;
			results[i][j].obj_min = 0.0;
			results[i][j].par_min.resize(in->num_srch_dof, 0.0);
		}
	}

}*/
/******************************************************************************/













