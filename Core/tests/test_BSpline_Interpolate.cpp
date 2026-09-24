#include <iostream>
#include <vector>
#include <cmath>
#include "Interpolate.h"
#include "Matrix_4d.h"
#include "test_utils.h"

void test_matrix_4d_bspline() {
    Matrix_4d mat(5, 5, 5);

    // Verify offsets and sizes
    ASSERT_EQ(mat.Bsp_offset(), 72);
    ASSERT_EQ(mat.siz_k(), 73);

    // Verify set_bsp and get_bsp
    mat.set_bsp(1, 2, 3, 123.456);
    ASSERT_NEAR(mat.get_bsp(1, 2, 3), 123.456, 1e-12);

    mat.set_bsp(0, 0, 0, -42.0);
    ASSERT_NEAR(mat.get_bsp(0, 0, 0), -42.0, 1e-12);

    mat.set_bsp(4, 4, 4, 999.99);
    ASSERT_NEAR(mat.get_bsp(4, 4, 4), 999.99, 1e-12);
}

void test_bspline_construction_and_validation() {
    Point pmin(0.0, 0.0, 0.0);
    Point pmax(20.0, 20.0, 20.0);
    BoundBox region(pmin, pmax);

    // Valid constructor calls
    {
        Interpolate interp3(&region, 3);
        ASSERT_EQ(interp3.bspline_order(), 3);
        ASSERT_FALSE(interp3.bspline_ready());
    }
    {
        Interpolate interp5(&region, 5);
        ASSERT_EQ(interp5.bspline_order(), 5);
        ASSERT_FALSE(interp5.bspline_ready());
    }
    {
        Interpolate interp7(&region, 7);
        ASSERT_EQ(interp7.bspline_order(), 7);
        ASSERT_FALSE(interp7.bspline_ready());
    }

    // Invalid constructor calls (should throw Intrp_Fail)
    ASSERT_THROWS(Interpolate(&region, 0), Intrp_Fail);
    ASSERT_THROWS(Interpolate(&region, 1), Intrp_Fail);
    ASSERT_THROWS(Interpolate(&region, 2), Intrp_Fail);
    ASSERT_THROWS(Interpolate(&region, 4), Intrp_Fail);
    ASSERT_THROWS(Interpolate(&region, 6), Intrp_Fail);
    ASSERT_THROWS(Interpolate(&region, 8), Intrp_Fail);

    // Single-argument constructor: not configured for bspline
    {
        Interpolate interp_legacy(&region);
        ASSERT_EQ(interp_legacy.bspline_order(), 0);
        ASSERT_FALSE(interp_legacy.bspline_ready());
        ASSERT_THROWS(interp_legacy.kernels_bspline(), Intrp_Fail);
    }
}

void test_set_bspline_order() {
    Point pmin(0.0, 0.0, 0.0);
    Point pmax(20.0, 20.0, 20.0);
    BoundBox region(pmin, pmax);

    // Constructed with order 7 (halo reserved = (7+1)/2 = 4)
    Interpolate interp7(&region, 7);
    ASSERT_EQ(interp7.bspline_order(), 7);

    // Downgrade to 5 (requires halo 3 <= 4)
    ASSERT_NO_THROW(interp7.set_bspline_order(5));
    ASSERT_EQ(interp7.bspline_order(), 5);
    ASSERT_FALSE(interp7.bspline_ready());

    // Downgrade to 3 (requires halo 2 <= 4)
    ASSERT_NO_THROW(interp7.set_bspline_order(3));
    ASSERT_EQ(interp7.bspline_order(), 3);

    // Upgrade back to 7 (requires halo 4 <= 4)
    ASSERT_NO_THROW(interp7.set_bspline_order(7));
    ASSERT_EQ(interp7.bspline_order(), 7);

    // Invalid orders throw Intrp_Fail
    ASSERT_THROWS(interp7.set_bspline_order(4), Intrp_Fail);
    ASSERT_THROWS(interp7.set_bspline_order(9), Intrp_Fail);

    // Constructed with order 3 (halo reserved = 2)
    Interpolate interp3(&region, 3);
    // Upgrading to order 5 requires halo 3 > 2 -> must throw Intrp_Fail
    ASSERT_THROWS(interp3.set_bspline_order(5), Intrp_Fail);
    ASSERT_THROWS(interp3.set_bspline_order(7), Intrp_Fail);
}

void test_staleness_and_interpolation() {
    // Generate synthetic volume: 30 x 30 x 30
    const int nx = 30, ny = 30, nz = 30;
    const double tolerance = 1e-3;
    const std::string vol_file = "tmp_synth_vol.raw";

    // Known smooth linear function f(x, y, z) = 10.0 + 1.0*x + 2.0*y + 3.0*z
    // B-spline interpolation reproduces linear polynomials with high precision
    // in the interior of the domain, away from boundary mirror artifacts.
    auto synth_fn = [](double x, double y, double z) -> double {
        return 10.0 + 1.0 * x + 2.0 * y + 3.0 * z;
    };

    bool written = write_synthetic_raw_volume(vol_file, nx, ny, nz, 0, 1, synth_fn);
    ASSERT_TRUE(written);

    Point vox_min(0.0, 0.0, 0.0);
    Point vox_max(nx, ny, nz);
    BoundBox vox_box(vox_min, vox_max);

    Point reg_min(2.0, 2.0, 2.0);
    Point reg_max(28.0, 28.0, 28.0);
    BoundBox region(reg_min, reg_max);

    // Construct Interpolate with order 7 (can also test 3 and 5)
    Interpolate interp(&region, 7);
    ASSERT_FALSE(interp.bspline_ready());

    // Query before kernels_bspline must throw Intrp_Fail
    std::vector<Point> test_pts = {Point(15.0, 15.0, 15.0)};
    BoundBox pt_bbox(Point(14.9, 14.9, 14.9), Point(15.1, 15.1, 15.1));
    std::vector<double> ivals(1, 0.0);
    std::vector<double> dfdx(1, 0.0), dfdy(1, 0.0), dfdz(1, 0.0);

    ASSERT_THROWS(interp.tri_bspline(test_pts, &pt_bbox, ivals), Intrp_Fail);
    ASSERT_THROWS(interp.tri_bspline_grad(test_pts, &pt_bbox, ivals, dfdx, dfdy, dfdz), Intrp_Fail);

    // Load voxel kernels
    interp.kernels(vol_file, &vox_box, 1, "little");
    // Verify bsp_valid is false after kernels() load
    ASSERT_FALSE(interp.bspline_ready());

    // Run kernels_bspline()
    interp.kernels_bspline();
    ASSERT_TRUE(interp.bspline_ready());

    // Test orders 3, 5, 7 on linear function:
    std::vector<int> orders = {3, 5, 7};
    for (int ord : orders) {
        interp.set_bspline_order(ord);
        ASSERT_FALSE(interp.bspline_ready());
        interp.kernels_bspline();
        ASSERT_TRUE(interp.bspline_ready());

        // Test points in deep interior of active region (away from mirror boundary effects)
        std::vector<Point> qpts = {
            Point(13.5, 14.2, 15.7),
            Point(15.0, 15.0, 15.0),
            Point(16.3, 15.8, 14.2)
        };
        BoundBox qbox(Point(13.0, 13.0, 13.0), Point(17.0, 17.0, 17.0));

        std::vector<double> qvals(qpts.size(), 0.0);
        interp.tri_bspline(qpts, &qbox, qvals);

        for (size_t i = 0; i < qpts.size(); i++) {
            double expected_val = synth_fn( qpts[i].x(), qpts[i].y(), qpts[i].z() );
            ASSERT_NEAR(qvals[i], expected_val, tolerance);
        }

        // Test tri_bspline_grad
        std::vector<double> gx(qpts.size(), 0.0), gy(qpts.size(), 0.0), gz(qpts.size(), 0.0);
        interp.tri_bspline_grad(qpts, &qbox, qvals, gx, gy, gz);

        for (size_t i = 0; i < qpts.size(); i++) {
            double expected_val = synth_fn( qpts[i].x(), qpts[i].y(), qpts[i].z() );
            ASSERT_NEAR(qvals[i], expected_val, tolerance);
            ASSERT_NEAR(gx[i], 1.0, tolerance);
            ASSERT_NEAR(gy[i], 2.0, tolerance);
            ASSERT_NEAR(gz[i], 3.0, tolerance);
        }
    }

    // Out of bounds query should throw Intrp_Fail
    std::vector<Point> oob_pts = {Point(0.5, 0.5, 0.5)};
    BoundBox oob_box(Point(0.0, 0.0, 0.0), Point(1.0, 1.0, 1.0));
    std::vector<double> oob_vals(1, 0.0);
    ASSERT_THROWS(interp.tri_bspline(oob_pts, &oob_box, oob_vals), Intrp_Fail);

    std::remove(vol_file.c_str());
}

int main() {
    std::cout << "=== Running BSpline Interpolate Tests ===" << std::endl;

    test_matrix_4d_bspline();
    test_bspline_construction_and_validation();
    test_set_bspline_order();
    test_staleness_and_interpolation();

    TEST_RUNNER_SUMMARY();
}

