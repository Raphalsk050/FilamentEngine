#pragma once

// Minimal test framework using CTest
// Each test is a function that returns 0 on success, non-zero on failure.
// Macros provide descriptive output on failure.

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <functional>

// Test registry for automatic test discovery within a single executable
struct TestCase {
    std::string name;
    std::function<void()> func;
};

inline std::vector<TestCase>& getTestRegistry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct TestRegistrar {
    TestRegistrar(const char* name, std::function<void()> func) {
        getTestRegistry().push_back({name, std::move(func)});
    }
};

// Register a test function
#define TEST(test_name) \
    void test_##test_name(); \
    static TestRegistrar registrar_##test_name(#test_name, test_##test_name); \
    void test_##test_name()

// Assertion macros
#define ASSERT_TRUE(expr) \
    do { \
        if (!(expr)) { \
            fprintf(stderr, "  FAIL: %s:%d: ASSERT_TRUE(%s) failed\n", __FILE__, __LINE__, #expr); \
            throw std::runtime_error("assertion failed"); \
        } \
    } while(0)

#define ASSERT_FALSE(expr) \
    do { \
        if ((expr)) { \
            fprintf(stderr, "  FAIL: %s:%d: ASSERT_FALSE(%s) failed\n", __FILE__, __LINE__, #expr); \
            throw std::runtime_error("assertion failed"); \
        } \
    } while(0)

#define ASSERT_EQ(a, b) \
    do { \
        if ((a) != (b)) { \
            fprintf(stderr, "  FAIL: %s:%d: ASSERT_EQ(%s, %s) failed\n", __FILE__, __LINE__, #a, #b); \
            throw std::runtime_error("assertion failed"); \
        } \
    } while(0)

#define ASSERT_NE(a, b) \
    do { \
        if ((a) == (b)) { \
            fprintf(stderr, "  FAIL: %s:%d: ASSERT_NE(%s, %s) failed\n", __FILE__, __LINE__, #a, #b); \
            throw std::runtime_error("assertion failed"); \
        } \
    } while(0)

#define ASSERT_LT(a, b) \
    do { \
        if (!((a) < (b))) { \
            fprintf(stderr, "  FAIL: %s:%d: ASSERT_LT(%s, %s) failed\n", __FILE__, __LINE__, #a, #b); \
            throw std::runtime_error("assertion failed"); \
        } \
    } while(0)

#define ASSERT_GT(a, b) \
    do { \
        if (!((a) > (b))) { \
            fprintf(stderr, "  FAIL: %s:%d: ASSERT_GT(%s, %s) failed\n", __FILE__, __LINE__, #a, #b); \
            throw std::runtime_error("assertion failed"); \
        } \
    } while(0)

#define ASSERT_NEAR(a, b, epsilon) \
    do { \
        if (std::fabs((a) - (b)) > (epsilon)) { \
            fprintf(stderr, "  FAIL: %s:%d: ASSERT_NEAR(%s, %s, %s) failed — got %f vs %f\n", \
                    __FILE__, __LINE__, #a, #b, #epsilon, (double)(a), (double)(b)); \
            throw std::runtime_error("assertion failed"); \
        } \
    } while(0)

#define ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == nullptr) { \
            fprintf(stderr, "  FAIL: %s:%d: ASSERT_NOT_NULL(%s) failed\n", __FILE__, __LINE__, #ptr); \
            throw std::runtime_error("assertion failed"); \
        } \
    } while(0)

#define ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != nullptr) { \
            fprintf(stderr, "  FAIL: %s:%d: ASSERT_NULL(%s) failed\n", __FILE__, __LINE__, #ptr); \
            throw std::runtime_error("assertion failed"); \
        } \
    } while(0)

// Main function that runs all registered tests
inline int runAllTests() {
    auto& tests = getTestRegistry();
    int passed = 0;
    int failed = 0;

    printf("Running %zu test(s)...\n\n", tests.size());

    for (auto& test : tests) {
        printf("[ RUN  ] %s\n", test.name.c_str());
        try {
            test.func();
            printf("[ PASS ] %s\n", test.name.c_str());
            passed++;
        } catch (const std::exception& e) {
            printf("[ FAIL ] %s\n", test.name.c_str());
            failed++;
        }
    }

    printf("\n========================================\n");
    printf("Results: %d passed, %d failed, %zu total\n", passed, failed, tests.size());
    printf("========================================\n");

    return failed > 0 ? 1 : 0;
}
