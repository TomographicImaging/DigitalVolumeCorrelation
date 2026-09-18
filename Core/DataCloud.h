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

// added for kdtree code
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <vector>
//
/******************************************************************************/

// kdtree code
struct Point3D {
    double x, y, z;
    int index;  // Original point index
};

struct Neighbor {
    int index;
    double distanceSquared;

    // For max-heap: largest distance is at the top
    bool operator<(const Neighbor& other) const {
        return distanceSquared < other.distanceSquared;
    }
};

class KDTree {
private:

    struct Node {
        int pointIndex;
        int axis;
        Node* left;
        Node* right;

        Node(int idx, int ax)
            : pointIndex(idx), axis(ax), left(nullptr), right(nullptr) {}
    };

    std::vector<Point3D> points;
    Node* root = nullptr;

    static double coord(const Point3D& p, int axis) {
        if (axis == 0) return p.x;
        if (axis == 1) return p.y;
        return p.z;
    }

    Node* build(std::vector<int>& indices, int depth) {

        if (indices.empty())
            return nullptr;

        int axis = depth % 3;

        size_t median = indices.size() / 2;

        std::nth_element(
            indices.begin(),
            indices.begin() + median,
            indices.end(),
            [&](int a, int b) {
                return coord(points[a], axis) <
                       coord(points[b], axis);
            }
        );

        int pointIndex = indices[median];

        Node* node = new Node(pointIndex, axis);

        std::vector<int> leftIndices(
            indices.begin(),
            indices.begin() + median
        );

        std::vector<int> rightIndices(
            indices.begin() + median + 1,
            indices.end()
        );

        node->left = build(leftIndices, depth + 1);
        node->right = build(rightIndices, depth + 1);

        return node;
    }

    static double distanceSquared(
        const Point3D& a,
        const Point3D& b)
    {
        double dx = a.x - b.x;
        double dy = a.y - b.y;
        double dz = a.z - b.z;

        return dx * dx + dy * dy + dz * dz;
    }

    void kNearestSearch(
        Node* node,
        const Point3D& query,
        int k,
        std::priority_queue<Neighbor>& heap)
    {
        if (!node)
            return;

        const Point3D& p = points[node->pointIndex];

        double dist2 = distanceSquared(query, p);

        // Include the query point itself
  		if ((int)heap.size() < k) {
    		heap.push({p.index, dist2});
		}
		else if (dist2 < heap.top().distanceSquared) {
    		heap.pop();
    		heap.push({p.index, dist2});
		}

        int axis = node->axis;

        double queryCoord = coord(query, axis);
        double nodeCoord = coord(p, axis);

        Node* nearChild;
        Node* farChild;

        if (queryCoord < nodeCoord) {
            nearChild = node->left;
            farChild = node->right;
        }
        else {
            nearChild = node->right;
            farChild = node->left;
        }

        // Search the closer side first
        kNearestSearch(
            nearChild,
            query,
            k,
            heap
        );

        // Determine whether we need to search the other side
        double planeDistance =
            queryCoord - nodeCoord;

        planeDistance *= planeDistance;

        if ((int)heap.size() < k ||
            planeDistance < heap.top().distanceSquared)
        {
            kNearestSearch(
                farChild,
                query,
                k,
                heap
            );
        }
    }

    void radiusSearch(
        Node* node,
        const Point3D& query,
        double radiusSquared,
        std::vector<Neighbor>& result)
    {
        if (!node)
            return;

        const Point3D& p = points[node->pointIndex];

        double dist2 = distanceSquared(query, p);

        if (p.index != query.index &&
            dist2 <= radiusSquared)
        {
            result.push_back({
                p.index,
                dist2
            });
        }

        int axis = node->axis;

        double queryCoord = coord(query, axis);
        double nodeCoord = coord(p, axis);

        double diff = queryCoord - nodeCoord;

        if (diff <= 0) {

            radiusSearch(
                node->left,
                query,
                radiusSquared,
                result
            );

            if (diff * diff <= radiusSquared) {
                radiusSearch(
                    node->right,
                    query,
                    radiusSquared,
                    result
                );
            }

        } else {

            radiusSearch(
                node->right,
                query,
                radiusSquared,
                result
            );

            if (diff * diff <= radiusSquared) {
                radiusSearch(
                    node->left,
                    query,
                    radiusSquared,
                    result
                );
            }
        }
    }

    void destroy(Node* node) {

        if (!node)
            return;

        destroy(node->left);
        destroy(node->right);

        delete node;
    }

public:

    explicit KDTree(const std::vector<Point3D>& input)
        : points(input)
    {
        std::vector<int> indices(points.size());

        for (size_t i = 0; i < points.size(); ++i)
            indices[i] = static_cast<int>(i);

        root = build(indices, 0);
    }

    ~KDTree() {
        destroy(root);
    }

    std::vector<Neighbor> kNearest(
        int pointIndex,
        int k)
    {
        std::priority_queue<Neighbor> heap;

        kNearestSearch(
            root,
            points[pointIndex],
            k,
            heap
        );

        std::vector<Neighbor> result;

        while (!heap.empty()) {
            result.push_back(heap.top());
            heap.pop();
        }

        // Heap gives farthest -> nearest.
        // Reverse it so result is nearest -> farthest.
        std::reverse(result.begin(), result.end());

        return result;
    }

    std::vector<Neighbor> radiusNeighbors(
        int pointIndex,
        double radius)
    {
        std::vector<Neighbor> result;

        radiusSearch(
            root,
            points[pointIndex],
            radius * radius,
            result
        );

        std::sort(
            result.begin(),
            result.end(),
            [](const Neighbor& a, const Neighbor& b) {
                return a.distanceSquared <
                       b.distanceSquared;
            }
        );

        return result;
    }
};


/******************************************************************************/
struct DualSort
{
	int index;
	double value;
};
/******************************************************************************/
// data for a single iteration
struct Iter_Stats
{
    int nits;

    double obj_beg;
    double obj_end;

    Point3D pos_beg;
    Point3D pos_end;
};
/******************************************************************************/
// Information from a completed search, used for cloud point neighborhoods
// Part of the opt starting point determination process
struct ResultRecord
{
	int status;		// as defined in Utility.h
	double obj_min;			// value at minumum
	std::vector<double> par_min;	// parameters at min [ndof]
    Iter_Stats iter_stats;     // iteration tracking data
};
/******************************************************************************/
// instantiated in dvc.cpp, initialized and organized in DataCloud::organize_cloud, passed as a pointer into Search::process_point
class CCPI_EXPORT DataCloud
{
public:
	DataCloud();
	
	void organize_cloud(RunControl *run);

	// sort cloud to establish point run order and neighbors (for starting points and strain calc)
	// needs points and labels already available, generates order and neigh
	void sort_order_neighbors(Point starting_point);

	// new version base don kd tree, much, much faster
	void sort_neighbors_kdtree(Point starting_point);
	
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
	
    // this is set-up for potentially multiple targets (correlate volumes) with updating in mind
    // ntrg is 1 for standard single correlate volume searches
	// vector of result records for a point
	std::vector< std::vector<ResultRecord> > results;	// [ntrg][npts]

    // storage for iteration summary data 
	//std::vector<std::vector<Iteration_Specs>> iter_track;

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







/******************************************************************************/

#endif
