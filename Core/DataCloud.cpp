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

	
}
/******************************************************************************/
void DataCloud::organize_cloud(RunControl *run)
{

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
	int neigh_num_save = 50;  // number of sorted neighbours saved for each point
	
	// just a quick check to avoid problems with really small test clouds
	if (points.size() < neigh_num_par) neigh_num_par = (int)points.size();
	
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
	std::cout << "sorting point cloud" << std::endl;
	neigh.resize(points.size());

	std::vector<std::vector<int>> save_neigh = {};
	save_neigh.resize(points.size());
	int N = neigh_num_save < indx_dist.size() ? neigh_num_save : indx_dist.size();
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
		for (int j = 0; j < neigh_num_par; j++)
			neigh[i].push_back(indx_dist_copy[j].index);
		
		
		for (int j = 0; j < neigh_num_par; j++)
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
					std::cout << "approx sorting: " << vi << " of " << neigh.size() << "\n";
				}
			}
		}
}
		
		// this loads varying numbers of elements based on proximity
//		for (int j=0; j<neigh.size(); j++)
//			if (indx_dist[j].value <= neigh_dst_par) neigh[i].push_back(indx_dist[j].index);
	
	}
	std::cout << "sorting finished, prepping for search ..." << std::endl;
    std::cout << "Saving sorted pointcloud" << std::endl;

    std::ofstream sorted_pc_file;
	sorted_pc_file.open(run->pts_fname + ".sorted");
	
	for (auto &x : save_neigh) {
		for (auto &k : x)
			sorted_pc_file << k << " ";
		sorted_pc_file << std::endl;
	}
	sorted_pc_file.close();

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
			results[i][j].par_min.resize(run->num_srch_dof, 0.0);
		}
	}

}
/******************************************************************************//******************************************************************************/
/*DataCloud::DataCloud (InputRead *in)
{

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
void DataCloud::split_by_volume (std::vector<DualSort> indx_dist, double vol_limit)
// assumes indx_dist sorted by distance from starting point at start
{
	
	int ngrp = 1;
	int grp = ngrp-1;
	
	sub_groups.resize(ngrp);
	sub_groups[grp].push_back(indx_dist[0].index);
	
	// sub_groups should really be a vector of Clouds with BoundBox's
	
/*	
	double min_x = points[indx_dist[0].index].x();
	double min_y = points[indx_dist[0].index].y();
	double min_z = points[indx_dist[0].index].z();
	
	double max_x = points[indx_dist[0].index].x();
	double max_y = points[indx_dist[0].index].y();
	double max_z = points[indx_dist[0].index].z();
*/
	for (int i=1; i<indx_dist.size(); i++) {
		
		double vol = points[sub_groups[grp][0]].pt_encl_vol(points[indx_dist[i].index]);
		
		
		// this is volume from first point to furthest, but not total volume of the group
		// need the volume of the box the group occupies
		
		if (vol <= vol_limit)
			sub_groups[grp].push_back(indx_dist[i].index);
			
		
		if (vol > vol_limit) {
			
			ngrp += 1;
			grp = ngrp-1;
			
			sub_groups.resize(ngrp);
			sub_groups[grp].push_back(indx_dist[i].index);
			
			for (int j=i; j<indx_dist.size(); j++)				
				indx_dist[j].value = points[indx_dist[i].index].pt_dist(points[indx_dist[j].index]);
			
			std::sort(indx_dist.begin()+i, indx_dist.end(), sortByValue);
			
		}
		
	}
	
}
/******************************************************************************/

/*
	if (new_point.x() < min_x) {min_x = new_point.x(); reset = true;}
	if (new_point.y() < min_y) {min_y = new_point.y(); reset = true;}
	if (new_point.z() < min_z) {min_z = new_point.z(); reset = true;}

	if (new_point.x() > max_x) {max_x = new_point.x(); reset = true;}
	if (new_point.y() > max_y) {max_y = new_point.y(); reset = true;}
	if (new_point.z() > max_z) {max_z = new_point.z(); reset = true;}


*/



















