#ifndef __TEST_FRAMEWORK_H__
#define __TEST_FRAMEWORK_H__

#include <stdio.h>
#include <stdbool.h>

// Test framework macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("    [FAIL] %s\n", message); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            printf("    [FAIL] %s (expected: %llu, actual: %llu)\n", \
                   message, (unsigned long long)(expected), (unsigned long long)(actual)); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(ptr, message) \
    do { \
        if ((ptr) == NULL) { \
            printf("    [FAIL] %s (pointer is NULL)\n", message); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_NULL(ptr, message) \
    do { \
        if ((ptr) != NULL) { \
            printf("    [FAIL] %s (pointer is not NULL)\n", message); \
            return false; \
        } \
    } while(0)

// Test result tracking
typedef struct {
    int total;
    int passed;
    int failed;
} test_results_t;

extern test_results_t test_results;

// Test declaration and execution
typedef bool (*test_func_t)(void);

#define RUN_TEST(test_name) \
    do { \
        printf("  Running %s...\n", #test_name); \
        test_results.total++; \
        if (test_name()) { \
            printf("  [PASS] %s\n", #test_name); \
            test_results.passed++; \
        } else { \
            printf("  [FAIL] %s\n", #test_name); \
            test_results.failed++; \
        } \
    } while(0)

#define PRINT_TEST_SUMMARY() \
    do { \
        printf("\n========================================\n"); \
        printf("Test Summary:\n"); \
        printf("  Total:  %d\n", test_results.total); \
        printf("  Passed: %d\n", test_results.passed); \
        printf("  Failed: %d\n", test_results.failed); \
        printf("========================================\n"); \
    } while(0)

#endif // __TEST_FRAMEWORK_H__
