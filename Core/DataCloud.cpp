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
void DataCloud::sort_order_neighbors(Point start_position)
// this is the brute-force version
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
		indx_dist[i].value = start_position.pt_dist(points[i]);
	}
	std::sort(indx_dist.begin(), indx_dist.end(), sortByValue);
	
	for (int i=0; i<points.size(); i++)
		order[i] = indx_dist[i].index;
	
	// *** get neighbors for each point	

	neigh.resize(points.size());
	std::vector<std::vector<int>> save_neigh = {};	// not sure what save_neigh does, perhaps old
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
void DataCloud::sort_neighbors_kdtree(Point start_position)
{
	std::cout << std::endl << "kdtree sorting ..." << std::endl;

	int neigh_num_save = nbr_num_save() < points.size() ? nbr_num_save() : points.size();

	// estblish search order based on distance from start_point
	// this is a standard search as the start_point is a 3d spatial coodinate, which may not be a cloud point
	
	order.resize(points.size());

	std::vector<DualSort> indx_dist(points.size());
	for (int i=0; i<points.size(); i++) {
		indx_dist[i].index = i;
		indx_dist[i].value = start_position.pt_dist(points[i]);
	}
	std::sort(indx_dist.begin(), indx_dist.end(), sortByValue);
	
	for (int i=0; i<points.size(); i++) {
		order[i] = indx_dist[i].index;
	}

	// use KDtree to find neighbor indices of all points
	
	// transfer point data into the kdtree Point3D structure
	std::vector<Point3D> cloud;
	for (int i = 0; i < (int)points.size(); ++i) {
		Point3D p;
		p.x = points[i].x();
		p.y = points[i].y();
		p.z = points[i].z();
		p.index = i;
		cloud.push_back(p);
	}

	// instantiate KDTree
	KDTree tree(cloud);

	// parse the nearest neighbor sorting
	neigh.resize(points.size());
	#pragma omp parallel for
	for (int i = 0; i < (int)points.size(); ++i) {

		// sort neighbors for each point in the cloud
        auto neighbors = tree.kNearest(i, neigh_num_save);

		// auto loop over neighbors, each element referenced as n, extract indices, store in neigh
        for (const auto& n : neighbors) {
			neigh[i].push_back(n.index);
        }
	}

	std::cout << "... finished" << std::endl;
}
/******************************************************************************/
void DataCloud::sort_neighbor_subsets()
	// setting up a process for cloud subset searches based on neighbors: neighbor_subset
	// the goal is more efficiency, with fewer and larger interpolators covering more points at a time 
	// (current Search needs some adjustment, move instantiation of interpolators into dvc.cpp and pointers into RunControl)
	// the concept is to break up the cloud into subsets based on the point neighbors
	// the logic:
	//	1. starting with point 1 in the run order, process n members of the local neighborhood (adjustable n for memory allocation capacity)
	//	2. instead of independant processing, create interpolators capable of processing the full subset
	//	3. as usual, all of these points are marked with a search result, removing the initial not_search designation
	//	4. move through the global list for the next point with not_search status (automatically adjacent to a group of points already run)
	//	5. within neighbors of this new point, process all unprocessed points within it's local neighborhood
	//	6. repeat
	//
	//	pre-sorting:
	//	1. this can be organized ahead of time by another cloud organization process following organize_cloud step
	//	2. create a neighbor_subset struct that contains:
	//		a. variable length vector of point indices to process
	//		b. a bbox that encompasses the point full point subset (used subsequently for interpolator instantiation)
	//	3. loop through the neighbor_subset list, loop through indices list, done
	//	4. the normal process of establishing starting point estimates from neighborhood informaiton is undisturbed
	//	5. the neighbor subsets and lists within can be established ahead of time through another sort neighbors type process
	//		a. go through the point list as described in logic creating a list of "already touched" points
	//		b. check this list , and do not include "already touched" points as new neighbor subsets are established

	// doesn't look like a good option ... in a random test with a 10000 pt rand cloud ...
	// number of unprocessed neighbors drops quickly to very low numbers
	// ended up with ~ 5000 neighbor subsets, many with 1 or two point, for trials of 75 and 27 and 8 neighbors considered
	// 
	//data.sort_neighbor_subsets();
{
// load std::vector<NeighborSubset> neighbor_subset using order

	// neighbor_subsets //

	//NeighborSubset a_nbr_sub;

	std::vector<int> touched;	// list of pseudo-processed point indices
	int point_index;
	bool untouched;

	int num_nbrs = 27;		// experimenting with less than the max

	for (int i=0; i<(int)neigh.size(); i++) {	// loop through points in procces order
//	for (int i=0; i<2; i++) {	// loop through points in procces order

		NeighborSubset a_nbr_sub;	// fresh copy of the struct

//		for (int j=0; j<(int)neigh[i].size(); j++) {	// loop through neighbors of points
		for (int j=0; j<num_nbrs; j++) {	// loop through neighbors of points

			point_index = neigh[order[i]][j];

			// check if point was a part of an earlier neighbor_subset, if not add to touched list

			untouched = true;
			for (int k=0; k<(int)touched.size(); k++) {
				if (point_index == touched[k]) {
					untouched = false;
					break;
				}
			}
			if (untouched == true) { 
				touched.push_back(point_index);
				a_nbr_sub.run_list.push_back(point_index);
			}
		}

		if (a_nbr_sub.run_list.size() != 0) {
			neighbor_subsets.push_back(a_nbr_sub);
		}

	}

	std::cout << std::endl << "neighbor_subsets.size() = " << neighbor_subsets.size() << std::endl;
//	for (int i=0; i<neighbor_subsets.size(); i++) {
//		std::cout << "list_size " << i << "= " << neighbor_subsets[i].run_list.size() << std::endl;
//	}

//	std::cout << "list_size 0 = " << neighbor_subsets[0].run_list.size() << std::endl;
//	std::cout << "list_size 1 = " << neighbor_subsets[1].run_list.size() << std::endl;

	// touched list now contains all of the points in the cloud

	//std::cout << std::endl << "neigh size " << (int)neigh.size() << " " << "number touched = " << (int)touched.size() << std::endl << std::endl;
//	std::cout << std::endl << "neighbor_subsets.size() = " << neighbor_subsets.size() << std::endl;
//	std::cout << "min_size = " << min_size << " " << "max_size = " << max_size << std::endl<< std::endl;

}
/******************************************************************************/

void DataCloud::organize_cloud(RunControl *run)
{
	// logic to determine if a starting point is given or I should use the default
	Point start_position = this->points[0];
	std::vector<double> nan_start_position = { std::nan(""), std::nan(""), std::nan("")  };
	if (nan_start_position != run->start_position) {
		start_position = Point(run->start_position[0], run->start_position[1], run->start_position[2]);
	}
	// establish point processing order and neighborhoods using brute force search (original method)
	//sort_order_neighbors(start_position);
	// write sort file
	//write_sort_file(run->pts_fname, neigh);

	// establish point processing order and neighborhoods using kdtree approach (much faster)
	sort_neighbors_kdtree(start_position);
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

	/**/
	//std::cout << std::endl << "results.size() = " << results.size() << std::endl;

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













