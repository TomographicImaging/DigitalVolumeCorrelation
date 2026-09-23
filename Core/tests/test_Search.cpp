#include <iostream>
#include <vector>
#include <cmath>
#include "Search.h"
#include "DataCloud.h"
#include "test_utils.h"

// Helper to configure a RunControl struct for testing Search
RunControl make_test_run_control(
    const std::string& ref_file,
    const std::string& cor_file,
    int nx, int ny, int nz,
    Objfcn_Type obj_type,
    bool bspline,
    int bspline_order,
    int ndof = 6)
{
    RunControl rc;
    rc.ref_fname = ref_file;
    rc.cor_fname = cor_file;
    rc.vol_wide = nx;
    rc.vol_high = ny;
    rc.vol_tall = nz;
    rc.vol_bit_depth = 8;
    rc.vol_hdr_lngth = 0;
    rc.vol_endian = "little";
    rc.sub_geo = sphere;
    rc.subvol_size = 6.0;
    rc.subvol_npts = 50;
    rc.subvol_aspect = {1.0, 1.0, 1.0};
    rc.disp_max = 2.0;
    rc.num_srch_dof = ndof;
    rc.obj_fcn = obj_type;
    rc.bspline = bspline;
    rc.bspline_order = bspline_order;
    rc.basin_radius = 0.0;
    rc.rigid_trans = {0.0, 0.0, 0.0};
    rc.int_typ = bspline ? (bspline_order == 3 ? tri_bspline_3 :
                           (bspline_order == 5 ? tri_bspline_5 : tri_bspline_7))
                         : tricubic;
    return rc;
}

void test_search_construction() {
    std::string ref_file = "tmp_search_ref.raw";
    std::string cor_file = "tmp_search_cor.raw";

    // Create 15x15x15 synthetic volumes
    write_synthetic_raw_volume(ref_file, 15, 15, 15, 0, 1, [](int x, int y, int z) {
        return 50.0 + x + y + z;
    });
    write_synthetic_raw_volume(cor_file, 15, 15, 15, 0, 1, [](int x, int y, int z) {
        return 50.0 + x + y + z;
    });

    // Test B-spline construction with orders 3, 5, 7
    for (int order : {3, 5, 7}) {
        RunControl rc = make_test_run_control(ref_file, cor_file, 15, 15, 15, ZNSSD, true, order);
        ASSERT_NO_THROW({
            Search s(&rc);
            ASSERT_EQ(s.interp->bspline_order(), order);
        });
    }

    // Test non-bspline construction
    {
        RunControl rc = make_test_run_control(ref_file, cor_file, 15, 15, 15, ZNSSD, false, 0);
        ASSERT_NO_THROW({
            Search s(&rc);
            ASSERT_EQ(s.interp->bspline_order(), 0);
        });
    }

    std::remove(ref_file.c_str());
    std::remove(cor_file.c_str());
}

void test_search_jacobian_and_obj_fcns() {
    const int nx = 25, ny = 25, nz = 25;
    std::string ref_file = "tmp_search_ref2.raw";
    std::string cor_file = "tmp_search_cor2.raw";

    // Known volume pattern
    write_synthetic_raw_volume(ref_file, nx, ny, nz, 0, 1, [](int x, int y, int z) {
        return 20.0 + 2.0 * x + 3.0 * y + 1.5 * z;
    });
    // Slight shift for correlate volume
    write_synthetic_raw_volume(cor_file, nx, ny, nz, 0, 1, [](int x, int y, int z) {
        return 20.0 + 2.0 * (x + 0.1) + 3.0 * y + 1.5 * z;
    });

    std::vector<Objfcn_Type> obj_types = {ZNSSD, ZSSD, NSSD, SSD, SAD};

    for (auto obj_type : obj_types) {
        RunControl rc = make_test_run_control(ref_file, cor_file, nx, ny, nz, obj_type, true, 3, 6);
        Search s(&rc);

        // Setup a search point at center (12, 12, 12)
        Point pt(12.0, 12.0, 12.0);
        s.fcld = new FloatingCloud(pt, s.subv_rad, s.subv_num, 1.0, 1.0, 1.0, 1);
        std::vector<ResultRecord> neigh_res;
        s.search_pt_setup(pt, neigh_res);

        // Test obj_val_at
        std::vector<double> a6(6, 0.0);
        double val = s.obj_val_at(a6);
        ASSERT_TRUE(val >= 0.0);

        // Test obj_val_at with residual return
        std::vector<double> res(s.subv_num, 0.0);
        double val_res = s.obj_val_at(a6, res);
        ASSERT_NEAR(val, val_res, 1e-9);
        ASSERT_EQ(res.size(), static_cast<size_t>(s.subv_num));

        // Test LM_prep_at (drives bspline_jacobian_at with 6 DOF)
        Eigen::VectorXd e(s.subv_num);
        Eigen::MatrixXd J(s.subv_num, 6);
        double lm_obj = s.LM_prep_at(a6, e, J);
        ASSERT_NEAR(lm_obj, val, 1e-9);

        // Check Jacobian matrix is populated (non-zero entries)
        bool has_non_zero = false;
        for (int r = 0; r < s.subv_num; r++) {
            for (int c = 0; c < 6; c++) {
                if (std::abs(J(r, c)) > 1e-12) {
                    has_non_zero = true;
                    break;
                }
            }
        }
        ASSERT_TRUE(has_non_zero);

        // Test Jacobian_at (public method)
        std::vector<std::vector<double>> J_mat(s.subv_num, std::vector<double>(6, 0.0));
        s.Jacobian_at(a6, J_mat);
        // Compare with J from LM_prep_at
        ASSERT_NEAR(J_mat[0][0], J(0, 0), 1e-9);

        delete s.fcld;
        s.fcld = nullptr;
    }

    std::remove(ref_file.c_str());
    std::remove(cor_file.c_str());
}

void test_search_trgrid_and_map_objective() {
    const int nx = 25, ny = 25, nz = 25;
    std::string ref_file = "tmp_search_ref3.raw";
    std::string cor_file = "tmp_search_cor3.raw";

    write_synthetic_raw_volume(ref_file, nx, ny, nz, 0, 1, [](int x, int y, int z) {
        return 30.0 + x + y + z;
    });
    write_synthetic_raw_volume(cor_file, nx, ny, nz, 0, 1, [](int x, int y, int z) {
        return 30.0 + x + y + z;
    });

    RunControl rc = make_test_run_control(ref_file, cor_file, nx, ny, nz, ZNSSD, true, 3, 3);
    rc.disp_max = 5.0;
    Search s(&rc);

    Point pt(12.0, 12.0, 12.0);
    s.fcld = new FloatingCloud(pt, s.subv_rad, s.subv_num, 1.0, 1.0, 1.0, 1);
    std::vector<ResultRecord> neigh_res;
    s.search_pt_setup(pt, neigh_res);

    // Test trgrid_global with basin_radius == 0.0 and out_as_raw == false (no-op early return)
    ASSERT_NO_THROW(s.trgrid_global(2.0, 0.0, 0, false));

    // Test trgrid_global with out_as_raw == true (writes global_echo.raw)
    std::remove("global_echo.raw");
    ASSERT_NO_THROW(s.trgrid_global(2.0, 2.0, 0, true));
    std::ifstream raw_file1("global_echo.raw", std::ios::binary);
    ASSERT_TRUE(raw_file1.is_open());
    raw_file1.close();
    std::remove("global_echo.raw");

    // Test map_objective_function directly with small grid (e.g. num_each_dim = 4)
    ASSERT_NO_THROW(s.map_objective_function(101, 1.0, 4));
    std::ifstream raw_file2("global_echo.raw", std::ios::binary);
    ASSERT_TRUE(raw_file2.is_open());
    raw_file2.close();
    std::remove("global_echo.raw");

    delete s.fcld;
    s.fcld = nullptr;

    std::remove(ref_file.c_str());
    std::remove(cor_file.c_str());
}

void test_search_process_point_bspline() {
    const int nx = 30, ny = 30, nz = 30;
    std::string ref_file = "tmp_search_ref4.raw";
    std::string cor_file = "tmp_search_cor4.raw";

    write_synthetic_raw_volume(ref_file, nx, ny, nz, 0, 1, [](int x, int y, int z) {
        return 30.0 + x + y + z;
    });
    write_synthetic_raw_volume(cor_file, nx, ny, nz, 0, 1, [](int x, int y, int z) {
        return 30.0 + x + y + z;
    });

    RunControl rc = make_test_run_control(ref_file, cor_file, nx, ny, nz, ZNSSD, true, 3, 3);
    rc.disp_max = 5.0;
    Search s(&rc);

    DataCloud dc;
    dc.points.push_back(Point(15.0, 15.0, 15.0));
    dc.labels.push_back(1);
    dc.neigh.resize(1);
    dc.results.resize(1);

    // Test process_point with map_flag = false
    // process_point ends by throwing Point_Good() on successful point processing
    ASSERT_THROWS(s.process_point(0, 0, false, 0, &dc), Point_Good);

    // Test process_point with map_flag = true but mismatched label (tests condition branch)
    ASSERT_THROWS(s.process_point(0, 0, true, 999, &dc), Point_Good);

    std::remove(ref_file.c_str());
    std::remove(cor_file.c_str());
}

void test_search_stream_operator() {
    std::string ref_file = "tmp_search_ref_stream.raw";
    std::string cor_file = "tmp_search_cor_stream.raw";

    write_synthetic_raw_volume(ref_file, 15, 15, 15, 0, 1, [](int x, int y, int z) { return 40.0; });
    write_synthetic_raw_volume(cor_file, 15, 15, 15, 0, 1, [](int x, int y, int z) { return 40.0; });

    std::vector<Objfcn_Type> objs = {SAD, SSD, ZSSD, NSSD, ZNSSD};
    std::vector<Interp_Type> interps = {nearest, trilinear, tricubic, tri_bspline_3, tri_bspline_5, tri_bspline_7};
    std::vector<Subvol_Type> geos = {cube, sphere};

    for (auto obj : objs) {
        for (auto interp : interps) {
            for (auto geo : geos) {
                RunControl rc = make_test_run_control(ref_file, cor_file, 15, 15, 15, obj, false, 0);
                rc.int_typ = interp;
                rc.sub_geo = geo;
                if (interp == tri_bspline_3) { rc.bspline = true; rc.bspline_order = 3; }
                else if (interp == tri_bspline_5) { rc.bspline = true; rc.bspline_order = 5; }
                else if (interp == tri_bspline_7) { rc.bspline = true; rc.bspline_order = 7; }
                else { rc.bspline = false; rc.bspline_order = 0; }

                Search s(&rc);
                std::ostringstream oss;
                oss << s;
                std::string str = oss.str();
                ASSERT_TRUE(str.find("Search settings:") != std::string::npos);
            }
        }
    }

    std::remove(ref_file.c_str());
    std::remove(cor_file.c_str());
}

void test_search_pt_setup_interp_types() {
    std::string ref_file = "tmp_search_ref_interp.raw";
    std::string cor_file = "tmp_search_cor_interp.raw";

    write_synthetic_raw_volume(ref_file, 20, 20, 20, 0, 1, [](int x, int y, int z) { return 30.0 + x; });
    write_synthetic_raw_volume(cor_file, 20, 20, 20, 0, 1, [](int x, int y, int z) { return 30.0 + x; });

    std::vector<Interp_Type> types = {nearest, trilinear, tricubic, tri_bspline_3};
    for (auto ityp : types) {
        bool is_bsp = (ityp == tri_bspline_3);
        int ord = is_bsp ? 3 : 0;
        RunControl rc = make_test_run_control(ref_file, cor_file, 20, 20, 20, ZNSSD, is_bsp, ord);
        rc.int_typ = ityp;
        rc.disp_max = 3.0;
        Search s(&rc);
        Point pt(10.0, 10.0, 10.0);
        s.fcld = new FloatingCloud(pt, s.subv_rad, s.subv_num, 1.0, 1.0, 1.0, 1);
        std::vector<ResultRecord> neigh_res;
        ASSERT_NO_THROW(s.search_pt_setup(pt, neigh_res));
        delete s.fcld;
        s.fcld = nullptr;
    }

    std::remove(ref_file.c_str());
    std::remove(cor_file.c_str());
}

int main() {
    std::cout << "=== Running Search Tests ===" << std::endl;

    test_search_construction();
    test_search_jacobian_and_obj_fcns();
    test_search_trgrid_and_map_objective();
    test_search_process_point_bspline();

    TEST_RUNNER_SUMMARY();
}

