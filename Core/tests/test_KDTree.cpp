#include <iostream>
#include <vector>
#include <cmath>
#include "DataCloud.h"
#include "test_utils.h"

// Test Neighbor operator<
void test_neighbor_ordering() {
    Neighbor n1{0, 4.0};
    Neighbor n2{1, 9.0};
    Neighbor n3{2, 1.0};

    // operator< compares distanceSquared
    ASSERT_TRUE(n1 < n2);
    ASSERT_FALSE(n2 < n1);
    ASSERT_TRUE(n3 < n1);
}

// Test empty and single-point KDTree
void test_kdtree_edge_cases() {
    // Empty KDTree
    std::vector<Point3D> empty_pts;
    KDTree empty_tree(empty_pts);

    // Single-point KDTree
    std::vector<Point3D> single_pts = {{1.0, 2.0, 3.0, 0}};
    KDTree single_tree(single_pts);

    auto kn = single_tree.kNearest(0, 1);
    ASSERT_EQ(kn.size(), 1u);
    ASSERT_EQ(kn[0].index, 0);
    ASSERT_NEAR(kn[0].distanceSquared, 0.0, 1e-12);

    auto rn = single_tree.radiusNeighbors(0, 5.0);
    // radiusNeighbors excludes query point itself (p.index != query.index)
    ASSERT_EQ(rn.size(), 0u);
}

// Test multi-point KDTree with kNearest and radiusNeighbors
void test_kdtree_search() {
    // Create a set of 3D points forming a cube and axes
    std::vector<Point3D> pts = {
        {0.0, 0.0, 0.0, 0},
        {1.0, 0.0, 0.0, 1},
        {0.0, 1.0, 0.0, 2},
        {0.0, 0.0, 1.0, 3},
        {1.0, 1.0, 0.0, 4},
        {1.0, 0.0, 1.0, 5},
        {0.0, 1.0, 1.0, 6},
        {1.0, 1.0, 1.0, 7},
        {2.0, 2.0, 2.0, 8},
        {-1.0, -1.0, -1.0, 9}
    };

    KDTree tree(pts);

    // kNearest: for point 0 (0,0,0), nearest should be point 0 with dist2=0
    auto kn1 = tree.kNearest(0, 1);
    ASSERT_EQ(kn1.size(), 1u);
    ASSERT_EQ(kn1[0].index, 0);
    ASSERT_NEAR(kn1[0].distanceSquared, 0.0, 1e-12);

    // kNearest with k=4
    // Distances from point 0:
    // pt 0: 0.0
    // pt 1, 2, 3: dist2 = 1.0
    auto kn4 = tree.kNearest(0, 4);
    ASSERT_EQ(kn4.size(), 4u);
    ASSERT_EQ(kn4[0].index, 0);
    ASSERT_NEAR(kn4[0].distanceSquared, 0.0, 1e-12);
    for (size_t i = 1; i < 4; i++) {
        ASSERT_NEAR(kn4[i].distanceSquared, 1.0, 1e-12);
    }

    // kNearest when k > number of points
    auto kn_all = tree.kNearest(0, 20);
    ASSERT_EQ(kn_all.size(), pts.size());
    // Verify distance monotonically increases
    for (size_t i = 1; i < kn_all.size(); i++) {
        ASSERT_TRUE(kn_all[i - 1].distanceSquared <= kn_all[i].distanceSquared);
    }

    // Test radiusNeighbors: radius = 1.1 around pt 0
    // Should include pts 1, 2, 3 (distance = 1.0 <= 1.21), but NOT pt 0
    auto rn = tree.radiusNeighbors(0, 1.1);
    ASSERT_EQ(rn.size(), 3u);
    for (const auto& nb : rn) {
        ASSERT_NEAR(nb.distanceSquared, 1.0, 1e-12);
        ASSERT_TRUE(nb.index == 1 || nb.index == 2 || nb.index == 3);
    }

    // radiusNeighbors from pt 9 (-1,-1,-1) with radius 0.5 (should find none)
    auto rn_empty = tree.radiusNeighbors(9, 0.5);
    ASSERT_EQ(rn_empty.size(), 0u);

    // radiusNeighbors from pt 7 (1,1,1) with radius = 1.05
    // Neighbors: pt 4 (1,1,0), pt 5 (1,0,1), pt 6 (0,1,1) all distance^2 = 1.0
    auto rn7 = tree.radiusNeighbors(7, 1.05);
    ASSERT_EQ(rn7.size(), 3u);

    // Query on points where queryCoord >= nodeCoord to hit other branch
    auto kn8 = tree.kNearest(8, 3);
    ASSERT_EQ(kn8.size(), 3u);
    ASSERT_EQ(kn8[0].index, 8); // (2,2,2)
    ASSERT_EQ(kn8[1].index, 7); // (1,1,1) dist2 = 3.0
}

// Test DataCloud::sort_neighbors_kdtree
void test_datacloud_sort_neighbors_kdtree() {
    DataCloud dc;

    // Create a regular 3x3x3 grid of points
    for (int z = 0; z < 3; z++) {
        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 3; x++) {
                dc.points.push_back(Point(x * 10.0, y * 10.0, z * 10.0));
                dc.labels.push_back(static_cast<int>(dc.points.size()));
            }
        }
    }

    Point start(0.0, 0.0, 0.0);
    dc.sort_neighbors_kdtree(start);

    ASSERT_EQ(dc.order.size(), dc.points.size());
    // Point closest to start (0,0,0) should be index 0
    ASSERT_EQ(dc.order[0], 0);

    // Check neigh structure
    ASSERT_EQ(dc.neigh.size(), dc.points.size());
    for (size_t i = 0; i < dc.neigh.size(); i++) {
        ASSERT_TRUE(dc.neigh[i].size() > 0);
        // The first neighbor of point i should be itself
        ASSERT_EQ(dc.neigh[i][0], static_cast<int>(i));
    }

    // Point 0's nearest neighbors after itself should be at distance 10
    // (indices corresponding to (10,0,0), (0,10,0), (0,0,10))
    std::vector<int> expected_neighbors_pt0 = {0};
    int n0_count = dc.neigh[0].size();
    ASSERT_TRUE(n0_count >= 4);
    for (int j = 1; j <= 3; j++) {
        int n_idx = dc.neigh[0][j];
        double d = dc.points[0].pt_dist(dc.points[n_idx]);
        ASSERT_NEAR(d, 10.0, 1e-6);
    }
}

int main() {
    std::cout << "=== Running KDTree Tests ===" << std::endl;

    test_neighbor_ordering();
    test_kdtree_edge_cases();
    test_kdtree_search();
    test_datacloud_sort_neighbors_kdtree();

    TEST_RUNNER_SUMMARY();
}

