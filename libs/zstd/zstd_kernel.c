// Kernel glue for the vendored zstd decoder (zstddeclib.c, see README.md).
// Its allocations go to the kernel heap, so it never includes stdlib.h.
#include <stddef.h>
#include <stdint.h>
#include "../../kernel/memory/kmalloc.h"
#include "../../kernel/drivers/serial.h"
#include "zstd_kernel.h"

static void* zstd_kernel_calloc(size_t count, size_t size) {
    if (size != 0 && count > (size_t)-1 / size) {
        return NULL;
    }
    uint8_t* ptr = kmalloc(count * size);
    if (ptr != NULL) {
        for (size_t i = 0; i < count * size; i++) {
            ptr[i] = 0;
        }
    }
    return ptr;
}

#define ZSTD_DEPS_MALLOC
#define ZSTD_malloc(s) kmalloc(s)
#define ZSTD_calloc(n, s) zstd_kernel_calloc((n), (s))
#define ZSTD_free(p) kfree(p)
#include "zstddeclib.c"

#include "zstd_test_vector.h"

static uint32_t zstd_test_rand(uint32_t* state) {
    *state = *state * 1103515245u + 12345u;
    return *state >> 16;
}

// Rebuilds the input of zstd_test_frame, keep in sync with make_test_vector.py
static void zstd_test_input(uint8_t* out, size_t size) {
    uint32_t state = 0x12345678;
    size_t pos = 0;
    while (pos < size) {
        if (pos < 256 || zstd_test_rand(&state) % 8 == 0) {
            // short run of literals from a 16 letter alphabet
            uint32_t count = 1 + zstd_test_rand(&state) % 16;
            for (uint32_t i = 0; i < count && pos < size; i++) {
                out[pos++] = 'a' + zstd_test_rand(&state) % 16;
            }
        } else {
            // match against earlier output, may overlap itself
            uint32_t length = 32 + zstd_test_rand(&state) % 480;
            uint32_t window = pos < 65536 ? (uint32_t)pos : 65536;
            uint32_t offset = 1 + zstd_test_rand(&state) % window;
            for (uint32_t i = 0; i < length && pos < size; i++) {
                out[pos] = out[pos - offset];
                pos++;
            }
        }
    }
}

int validate_zstd(void) {
    size_t frame_size = sizeof(zstd_test_frame);
    if (ZSTD_getFrameContentSize(zstd_test_frame, frame_size) != ZSTD_TEST_CONTENT_SIZE) {
        serial_writestring("ZSTD validation failed: wrong content size in the frame header\n");
        return -1;
    }

    uint8_t* expected = kmalloc(ZSTD_TEST_CONTENT_SIZE);
    uint8_t* decoded = kmalloc(ZSTD_TEST_CONTENT_SIZE);
    uint8_t* corrupted = kmalloc(frame_size);
    if (expected == NULL || decoded == NULL || corrupted == NULL) {
        serial_writestring("ZSTD validation failed: out of memory\n");
        return -1;
    }
    zstd_test_input(expected, ZSTD_TEST_CONTENT_SIZE);

    size_t result = ZSTD_decompress(decoded, ZSTD_TEST_CONTENT_SIZE, zstd_test_frame, frame_size);
    if (ZSTD_isError(result) || result != ZSTD_TEST_CONTENT_SIZE) {
        serial_writestring("ZSTD validation failed: the test frame did not decode\n");
        return -1;
    }
    for (size_t i = 0; i < ZSTD_TEST_CONTENT_SIZE; i++) {
        if (decoded[i] != expected[i]) {
            serial_writestring("ZSTD validation failed: decoded bytes differ from the input\n");
            return -1;
        }
    }

    // A flipped bit in the compressed data has to be caught, by the decoder or the frame checksum
    for (size_t i = 0; i < frame_size; i++) {
        corrupted[i] = zstd_test_frame[i];
    }
    corrupted[frame_size / 2] ^= 0x01;
    result = ZSTD_decompress(decoded, ZSTD_TEST_CONTENT_SIZE, corrupted, frame_size);
    if (!ZSTD_isError(result)) {
        serial_writestring("ZSTD validation failed: a corrupted frame was accepted\n");
        return -1;
    }

    kfree(corrupted);
    kfree(decoded);
    kfree(expected);
    serial_writestring("ZSTD validation passed!\n");
    return 0;
}
