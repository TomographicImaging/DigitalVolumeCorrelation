#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include "InputRead.h"
#include "test_utils.h"

// Helper to create char* array from std::vector<std::string>
struct ArgvHelper {
    std::vector<std::string> storage;
    std::vector<char*> ptrs;
    int argc;

    ArgvHelper(const std::vector<std::string>& args) : storage(args) {
        for (auto& s : storage) {
            ptrs.push_back(&s[0]);
        }
        ptrs.push_back(nullptr);
        argc = static_cast<int>(storage.size());
    }

    char** argv() { return ptrs.data(); }
};

void test_find_flag_bool() {
    InputRead in;

    // Flag present in middle
    {
        ArgvHelper args({"prog", "-v", "input.txt"});
        int ret = in.find_flag("-v", args.argc, args.argv());
        ASSERT_EQ(ret, 1);
        ASSERT_EQ(args.argc, 2);
        ASSERT_EQ(std::string(args.argv()[0]), "prog");
        ASSERT_EQ(std::string(args.argv()[1]), "input.txt");
    }

    // Flag present at end
    {
        ArgvHelper args({"prog", "input.txt", "-flag"});
        int ret = in.find_flag("-flag", args.argc, args.argv());
        ASSERT_EQ(ret, 1);
        ASSERT_EQ(args.argc, 2);
        ASSERT_EQ(std::string(args.argv()[0]), "prog");
        ASSERT_EQ(std::string(args.argv()[1]), "input.txt");
    }

    // Flag absent
    {
        ArgvHelper args({"prog", "input.txt"});
        int ret = in.find_flag("-absent", args.argc, args.argv());
        ASSERT_EQ(ret, 0);
        ASSERT_EQ(args.argc, 2);
    }
}

void test_find_flag_int() {
    InputRead in;

    // Valid int flag
    {
        ArgvHelper args({"prog", "-map", "42", "file.txt"});
        int val = 0;
        int ret = in.find_flag("-map", args.argc, args.argv(), val);
        ASSERT_EQ(ret, 1);
        ASSERT_EQ(val, 42);
        ASSERT_EQ(args.argc, 2);
        ASSERT_EQ(std::string(args.argv()[0]), "prog");
        ASSERT_EQ(std::string(args.argv()[1]), "file.txt");
    }

    // Invalid int argument
    {
        ArgvHelper args({"prog", "-map", "not_a_number", "file.txt"});
        int val = 0;
        int ret = in.find_flag("-map", args.argc, args.argv(), val);
        ASSERT_EQ(ret, 0);
        ASSERT_EQ(args.argc, 3);
        ASSERT_EQ(std::string(args.argv()[1]), "not_a_number");
    }

    // Flag at end without following argument
    {
        ArgvHelper args({"prog", "file.txt", "-map"});
        int val = 0;
        int ret = in.find_flag("-map", args.argc, args.argv(), val);
        ASSERT_EQ(ret, 0);
        ASSERT_EQ(args.argc, 2);
    }

    // Flag not found
    {
        ArgvHelper args({"prog", "file.txt"});
        int val = 0;
        int ret = in.find_flag("-map", args.argc, args.argv(), val);
        ASSERT_EQ(ret, 0);
        ASSERT_EQ(args.argc, 2);
    }
}

void test_find_flag_double() {
    InputRead in;

    // Valid double flag
    {
        ArgvHelper args({"prog", "-t", "3.1415", "file.txt"});
        double val = 0.0;
        int ret = in.find_flag("-t", args.argc, args.argv(), val);
        ASSERT_EQ(ret, 1);
        ASSERT_NEAR(val, 3.1415, 1e-6);
        ASSERT_EQ(args.argc, 2);
        ASSERT_EQ(std::string(args.argv()[0]), "prog");
        ASSERT_EQ(std::string(args.argv()[1]), "file.txt");
    }

    // Invalid double argument
    {
        ArgvHelper args({"prog", "-t", "bad_float", "file.txt"});
        double val = 0.0;
        int ret = in.find_flag("-t", args.argc, args.argv(), val);
        ASSERT_EQ(ret, 0);
        ASSERT_EQ(args.argc, 3);
        ASSERT_EQ(std::string(args.argv()[1]), "bad_float");
    }

    // Flag at end without following argument
    {
        ArgvHelper args({"prog", "file.txt", "-t"});
        double val = 0.0;
        int ret = in.find_flag("-t", args.argc, args.argv(), val);
        ASSERT_EQ(ret, 0);
        ASSERT_EQ(args.argc, 2);
    }

    // Flag not found
    {
        ArgvHelper args({"prog", "file.txt"});
        double val = 0.0;
        int ret = in.find_flag("-t", args.argc, args.argv(), val);
        ASSERT_EQ(ret, 0);
        ASSERT_EQ(args.argc, 2);
    }
}

void test_find_flag_prefix() {
    InputRead in;

    // Unused flag matching prefix "-" at pos 0 len 1
    {
        ArgvHelper args({"prog", "file1.txt", "-unused", "file2.txt"});
        int ret = in.find_flag(0, 1, "-", args.argc, args.argv());
        ASSERT_EQ(ret, 1);
        ASSERT_EQ(args.argc, 3);
        ASSERT_EQ(std::string(args.argv()[1]), "file1.txt");
        ASSERT_EQ(std::string(args.argv()[2]), "file2.txt");
    }

    // No unused flag matching prefix
    {
        ArgvHelper args({"prog", "file1.txt", "file2.txt"});
        int ret = in.find_flag(0, 1, "-", args.argc, args.argv());
        ASSERT_EQ(ret, 0);
        ASSERT_EQ(args.argc, 3);
    }
}

// Helper to create test input files by modifying datacloud_input.txt interp_type
bool create_input_from_template(const std::string& template_file,
                                const std::string& target_file,
                                const std::string& new_interp_type) {
    std::ifstream in(template_file.c_str());
    if (!in.is_open()) return false;

    std::ofstream out(target_file.c_str());
    if (!out.is_open()) return false;

    std::string line;
    while (std::getline(in, line)) {
        if (line.find("interp_type") != std::string::npos) {
            out << "interp_type\t" << new_interp_type << "\t### modified for test\n";
        } else {
            out << line << "\n";
        }
    }
    return true;
}

void test_input_file_read_bspline() {
    // Ensure reference, correlate, and roi files exist
    bool created_ref = false, created_cor = false, created_roi = false;
    std::ifstream cr("reference.npy");
    if (!cr.good()) {
        std::ofstream r("reference.npy", std::ios::binary);
        std::vector<char> dummy(96 + 20 * 30 * 40, 0);
        r.write(dummy.data(), dummy.size());
        created_ref = true;
    }
    std::ifstream cc("correlate.npy");
    if (!cc.good()) {
        std::ofstream c("correlate.npy", std::ios::binary);
        std::vector<char> dummy(96 + 20 * 30 * 40, 0);
        c.write(dummy.data(), dummy.size());
        created_cor = true;
    }
    std::ifstream cp("grid_input.roi");
    if (!cp.good()) {
        std::ofstream p("grid_input.roi");
        p << "0\t10\t10\t10\n";
        created_roi = true;
    }
    std::string template_file = "datacloud_input.txt";

    // Test tri_bspline_3
    {
        std::string fname = "tmp_input_bsp3.txt";
        bool ok = create_input_from_template(template_file, fname, "tri_bspline_3");
        ASSERT_TRUE(ok);
        if (ok) {
            InputRead in;
            ASSERT_TRUE(in.input_file_accessible(fname));
            RunControl run;
            int status = in.input_file_read(&run);
            ASSERT_EQ(status, 1);
            ASSERT_TRUE(run.bspline);
            ASSERT_EQ(run.bspline_order, 3);
            ASSERT_EQ(run.int_typ, tri_bspline_3);
            std::remove(fname.c_str());
        }
    }

    // Test tri_bspline_5
    {
        std::string fname = "tmp_input_bsp5.txt";
        bool ok = create_input_from_template(template_file, fname, "tri_bspline_5");
        ASSERT_TRUE(ok);
        if (ok) {
            InputRead in;
            ASSERT_TRUE(in.input_file_accessible(fname));
            RunControl run;
            int status = in.input_file_read(&run);
            ASSERT_EQ(status, 1);
            ASSERT_TRUE(run.bspline);
            ASSERT_EQ(run.bspline_order, 5);
            ASSERT_EQ(run.int_typ, tri_bspline_5);
            std::remove(fname.c_str());
        }
    }

    // Test tri_bspline_7
    {
        std::string fname = "tmp_input_bsp7.txt";
        bool ok = create_input_from_template(template_file, fname, "tri_bspline_7");
        ASSERT_TRUE(ok);
        if (ok) {
            InputRead in;
            ASSERT_TRUE(in.input_file_accessible(fname));
            RunControl run;
            int status = in.input_file_read(&run);
            ASSERT_EQ(status, 1);
            ASSERT_TRUE(run.bspline);
            ASSERT_EQ(run.bspline_order, 7);
            ASSERT_EQ(run.int_typ, tri_bspline_7);
            std::remove(fname.c_str());
        }
    }

    // Test non-bspline (tricubic)
    {
        std::string fname = "tmp_input_tricubic.txt";
        bool ok = create_input_from_template(template_file, fname, "tricubic");
        ASSERT_TRUE(ok);
        if (ok) {
            InputRead in;
            ASSERT_TRUE(in.input_file_accessible(fname));
            RunControl run;
            int status = in.input_file_read(&run);
            ASSERT_EQ(status, 1);
            ASSERT_FALSE(run.bspline);
            ASSERT_EQ(run.bspline_order, 0);
            ASSERT_EQ(run.int_typ, tricubic);
            std::remove(fname.c_str());
        }
    }

    if (created_ref) std::remove("reference.npy");
    if (created_cor) std::remove("correlate.npy");
    if (created_roi) std::remove("grid_input.roi");
}

void test_append_result() {
    InputRead in;
    std::string fname = "tmp_test_res.disp";
    std::remove(fname.c_str());

    Point pt(1.2345, 6.7891, 2.3456);
    std::vector<double> result = {0.123456, -0.654321, 1.111111};
    int ret = in.append_result(fname, 1, pt, 0, 0.05, result);
    ASSERT_EQ(ret, 1);

    // Read back and check contents
    std::ifstream in_file(fname.c_str());
    ASSERT_TRUE(in_file.is_open());
    int n, status;
    double x, y, z, obj_min, u, v, w;
    in_file >> n >> x >> y >> z >> status >> obj_min >> u >> v >> w;
    ASSERT_EQ(n, 1);
    ASSERT_NEAR(x, 1.234, 1e-2);
    ASSERT_NEAR(y, 6.789, 1e-2);
    ASSERT_NEAR(z, 2.346, 1e-2);
    ASSERT_EQ(status, 0);
    ASSERT_NEAR(obj_min, 0.05, 1e-6);
    ASSERT_NEAR(u, 0.123456, 1e-6);
    ASSERT_NEAR(v, -0.654321, 1e-6);
    ASSERT_NEAR(w, 1.111111, 1e-6);
    in_file.close();
    std::remove(fname.c_str());
}

int main() {
    std::cout << "=== Running InputRead Tests ===" << std::endl;

    test_find_flag_bool();
    test_find_flag_int();
    test_find_flag_double();
    test_find_flag_prefix();
    test_input_file_read_bspline();
    test_append_result();

    TEST_RUNNER_SUMMARY();
}

