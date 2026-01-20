#include "mock_serial.h"
#include <stdio.h>

// Mock implementation that prints to stdout for testing
void serial_writestring(const char* str) {
    printf("%s", str);
}
