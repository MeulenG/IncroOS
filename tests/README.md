# IncroOS Memory Manager Test Suite

This directory contains unit tests for the IncroOS memory management subsystem.

## Overview

The test suite validates the functionality of the following memory managers:
- **kmalloc**: Kernel heap allocator
- **PMM**: Physical Memory Manager (page allocator)

## Building the Tests

To build all tests:
```bash
cd tests
make
```

## Running the Tests

To run all tests:
```bash
cd tests
make run
```

To run individual tests:
```bash
./test_kmalloc
./test_pmm
```

## Test Coverage

### kmalloc Tests
- Initialization
- Basic allocation
- Zero-size allocation
- NULL pointer handling
- Allocation and deallocation
- Multiple allocations
- Memory reuse
- 16-byte alignment
- Fragmentation handling
- Block coalescing
- Memory tracking (used/free)

### PMM Tests
- Initialization
- Page allocation
- Page deallocation
- Multiple allocations
- Page reuse
- Page tracking
- Invalid address handling
- Double-free protection
- Page alignment

## Test Framework

The tests use a simple custom test framework (`test_framework.h`) that provides:
- `TEST_ASSERT`: Assert a condition is true
- `TEST_ASSERT_EQUAL`: Assert two values are equal
- `TEST_ASSERT_NOT_NULL`: Assert a pointer is not NULL
- `TEST_ASSERT_NULL`: Assert a pointer is NULL
- `RUN_TEST`: Execute a test and track results
- `PRINT_TEST_SUMMARY`: Display test results

## Implementation Notes

The tests run in a hosted (userspace) environment rather than the bare-metal kernel environment. This is achieved by:
- Using standard C library functions (malloc/free) to simulate kernel memory regions
- Mocking the serial driver output
- Compiling with standard GCC rather than the freestanding kernel build

This approach allows for rapid test development and execution without requiring the full OS boot process.

## Adding New Tests

To add new tests:

1. Create a new test file (e.g., `test_vmm.c`)
2. Include `test_framework.h`
3. Implement test functions that return `bool`
4. Use `TEST_ASSERT*` macros to verify behavior
5. Add a `main()` function that calls `RUN_TEST()` for each test
6. Update the Makefile to build the new test
7. Run `make run` to verify all tests pass

Example:
```c
#include "test_framework.h"

bool test_my_feature(void) {
    // Test implementation
    TEST_ASSERT(condition, "condition should be true");
    return true;
}

int main(void) {
    RUN_TEST(test_my_feature);
    PRINT_TEST_SUMMARY();
    return (test_results.failed == 0) ? 0 : 1;
}
```
