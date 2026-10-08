#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>
#include <functional>
#include <cstdlib>

static int g_test_count = 0;
static int g_test_failures = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        g_test_count++; \
        if (!(cond)) { \
            std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__ \
                      << " - " << msg << std::endl; \
            g_test_failures++; \
        } \
    } while (0)

#define ASSERT_TRUE(cond) TEST_ASSERT((cond), #cond " is false")
#define ASSERT_FALSE(cond) TEST_ASSERT(!(cond), #cond " is true")

#define ASSERT_EQ(a, b) \
    do { \
        auto val_a = (a); \
        auto val_b = (b); \
        TEST_ASSERT(val_a == val_b, #a " (" << val_a << ") != " #b " (" << val_b << ")"); \
    } while (0)

#define ASSERT_NEAR(a, b, tol) \
    do { \
        double diff = std::abs((double)(a) - (double)(b)); \
        TEST_ASSERT(diff <= (double)(tol), \
            #a " (" << (a) << ") and " #b " (" << (b) << ") differ by " << diff << " > tol=" << (tol)); \
    } while (0)

#define ASSERT_THROWS(expr, ExceptionType) \
    do { \
        g_test_count++; \
        bool threw = false; \
        try { \
            expr; \
        } catch (const ExceptionType&) { \
            threw = true; \
        } catch (...) { \
            threw = false; \
        } \
        if (!threw) { \
            std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__ \
                      << " - Expected " #expr " to throw " #ExceptionType << std::endl; \
            g_test_failures++; \
        } \
    } while (0)

#define ASSERT_NO_THROW(expr) \
    do { \
        g_test_count++; \
        bool threw = false; \
        try { \
            expr; \
        } catch (...) { \
            threw = true; \
        } \
        if (threw) { \
            std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__ \
                      << " - " #expr " threw unexpected exception" << std::endl; \
            g_test_failures++; \
        } \
    } while (0)

#define TEST_RUNNER_SUMMARY() \
    do { \
        std::cout << "\nRan " << g_test_count << " assertions: " \
                  << (g_test_count - g_test_failures) << " passed, " \
                  << g_test_failures << " failed." << std::endl; \
        return (g_test_failures == 0) ? 0 : 1; \
    } while (0)

// Helper to write synthetic voxel volumes
inline bool write_synthetic_raw_volume(
    const std::string& filename,
    int nx, int ny, int nz,
    int header_bytes,
    int bytes_per_voxel,
    std::function<double(int x, int y, int z)> func)
{
    std::ofstream out(filename.c_str(), std::ios::binary);
    if (!out.is_open()) return false;

    // Header padding if requested
    if (header_bytes > 0) {
        std::vector<char> hdr(header_bytes, 0);
        out.write(hdr.data(), header_bytes);
    }

    // Voxel data in C/row-major or slice-major order:
    // DVC convention: ic (x) fastest, ir (y) middle, is (z) slowest
    for (int z = 0; z < nz; z++) {
        for (int y = 0; y < ny; y++) {
            for (int x = 0; x < nx; x++) {
                double val = func(x, y, z);
                if (bytes_per_voxel == 1) {
                    unsigned char b = static_cast<unsigned char>(
                        std::max(0.0, std::min(255.0, val)));
                    out.write(reinterpret_cast<char*>(&b), 1);
                } else if (bytes_per_voxel == 2) {
                    unsigned short s = static_cast<unsigned short>(
                        std::max(0.0, std::min(65535.0, val)));
                    out.write(reinterpret_cast<char*>(&s), 2);
                } else {
                    float f = static_cast<float>(val);
                    out.write(reinterpret_cast<char*>(&f), sizeof(float));
                }
            }
        }
    }
    return true;
}

#endif // TEST_UTILS_H

