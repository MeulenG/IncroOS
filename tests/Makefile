# Test suite Makefile

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -g -O0
LDFLAGS = 

# Test executables
TEST_KMALLOC = test_kmalloc
TEST_PMM = test_pmm

# All tests
TESTS = $(TEST_KMALLOC) $(TEST_PMM)

.PHONY: all clean run

all: $(TESTS)

# Build kmalloc tests
$(TEST_KMALLOC): test_kmalloc.c test_framework.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Build PMM tests
$(TEST_PMM): test_pmm.c test_framework.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Run all tests
run: all
	@echo "Running all memory manager tests..."
	@echo ""
	@./$(TEST_KMALLOC) || exit 1
	@echo ""
	@./$(TEST_PMM) || exit 1
	@echo ""
	@echo "All tests completed successfully!"

clean:
	rm -f $(TESTS) *.o
