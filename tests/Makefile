# Test suite Makefile using Unity framework

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -g -O0 -Iunity
LDFLAGS = 

# Unity sources
UNITY_SRC = unity/unity.c

# Test executables
TEST_KMALLOC = test_kmalloc
TEST_PMM = test_pmm

# All tests
TESTS = $(TEST_KMALLOC) $(TEST_PMM)

.PHONY: all clean run

all: $(TESTS)

# Build kmalloc tests
$(TEST_KMALLOC): test_kmalloc.c $(UNITY_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Build PMM tests
$(TEST_PMM): test_pmm.c $(UNITY_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Run all tests
run: all
	@echo "Running all memory manager tests with Unity framework..."
	@echo ""
	@./$(TEST_KMALLOC)
	@echo ""
	@./$(TEST_PMM)
	@echo ""
	@echo "All tests completed!"

clean:
	rm -f $(TESTS) *.o
