# IncroOS Memory Manager Test Suite

This directory contains unit tests for the IncroOS memory management subsystem using the Unity test framework.

## Overview

The test suite validates the functionality of the following memory managers:
- **kmalloc**: Kernel heap allocator
- **PMM**: Physical Memory Manager (page allocator)

## Test Framework

The tests use the [Unity Test Framework](https://github.com/ThrowTheSwitch/Unity), a lightweight C unit testing framework designed for embedded systems. Unity provides:
- Rich assertion macros (TEST_ASSERT_EQUAL, TEST_ASSERT_NOT_NULL, etc.)
- Clear test output with pass/fail indicators
- Automatic test counting and reporting
- Minimal dependencies and overhead

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

### kmalloc Tests (11 tests)
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

### PMM Tests (9 tests)
- Initialization
- Page allocation
- Page deallocation
- Multiple allocations
- Page reuse
- Page tracking
- Invalid address handling
- Double-free protection
- Page alignment

## Implementation Notes

The tests run in a hosted (userspace) environment rather than the bare-metal kernel environment. This is achieved by:
- Using standard C library functions (malloc/free) to simulate kernel memory regions
- Mocking the serial driver output
- Compiling with standard GCC rather than the freestanding kernel build

This approach allows for rapid test development and execution without requiring the full OS boot process.

## Unity Framework

Unity is included in the `unity/` directory. The framework consists of:
- `unity.c` - Main implementation
- `unity.h` - Public API
- `unity_internals.h` - Internal definitions

Unity is licensed under the MIT License and is maintained by ThrowTheSwitch.org.

## Adding New Tests

To add new tests:

1. Create a new test file (e.g., `test_vmm.c`)
2. Include `unity/unity.h`
3. Implement test functions with void return type
4. Use Unity's TEST_ASSERT* macros
5. Implement setUp() and tearDown() for test initialization/cleanup
6. Add RUN_TEST() calls in main()
7. Update the Makefile to build the new test
8. Run `make run` to verify all tests pass

Example:
```c
#include "unity/unity.h"

void setUp(void) {
    // Initialize test environment
}

void tearDown(void) {
    // Cleanup after test
}

void test_my_feature(void) {
    TEST_ASSERT_EQUAL(expected, actual);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_my_feature);
    return UNITY_END();
}
```

## Clean Build

To remove all built test executables:
```bash
cd tests
make clean
```
