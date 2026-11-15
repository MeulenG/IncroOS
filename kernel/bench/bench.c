#include "bench.h"
#include "../memory/pmm.h"
#include "../memory/vmm.h"
#include "../memory/kmalloc.h"
#include "../drivers/vesa.h"
#include "../../libs/libkernel/print.h"

static void report(const char *name, uint64_t cycles) {
    kprintf("[BENCH] %s: %lu\n", name, cycles);
}

// Decimal digits into out, NUL-terminated; the length
static int format_u64(uint64_t value, char *out) {
    char temp[20];
    int n = 0;
    do {
        temp[n++] = (char)('0' + value % 10);
        value /= 10;
    } while (value > 0);
    int i = 0;
    while (n > 0) {
        out[i++] = temp[--n];
    }
    out[i] = '\0';
    return i;
}

void run_benchmarks(uint64_t boot_cycles) {
    uint64_t checksum = 0;
    report("boot services", boot_cycles);

    // PMM: 512 pages allocated, then freed (first fit from page 0)
    static uint64_t pages[512];
    uint64_t start = bench_now();
    for (int i = 0; i < 512; i++) {
        pages[i] = pmm_alloc_page();
    }
    for (int i = 0; i < 512; i++) {
        pmm_free_page(pages[i]);
    }
    report("pmm 512 alloc+free", bench_now() - start);

    // VMM: 256 pages at 8 GiB mapped, looked up, unmapped
    const uint64_t base = 0x200000000ULL;
    start = bench_now();
    for (uint64_t i = 0; i < 256; i++) {
        vmm_map_page(base + i * PAGE_SIZE, pages[i], PT_WRITABLE);
    }
    for (uint64_t i = 0; i < 256; i++) {
        checksum += vmm_get_physical(base + i * PAGE_SIZE);
    }
    for (uint64_t i = 0; i < 256; i++) {
        vmm_unmap_page(base + i * PAGE_SIZE);
    }
    report("vmm 256 map+lookup+unmap", bench_now() - start);

    // kmalloc: 1000 blocks of 64 bytes, freed in reverse
    static void *blocks[1000];
    start = bench_now();
    for (int i = 0; i < 1000; i++) {
        blocks[i] = kmalloc(64);
    }
    for (int i = 999; i >= 0; i--) {
        kfree(blocks[i]);
    }
    report("kmalloc 1000x64 alloc+free", bench_now() - start);

    // kmalloc: 2000 blocks of 24 bytes, kept
    start = bench_now();
    for (int i = 0; i < 2000; i++) {
        checksum += (uint64_t)(kmalloc(24) != NULL);
    }
    report("kmalloc 2000x24 kept", bench_now() - start);

    // Formatting: 20000 64-bit numbers to decimal
    char text[24];
    start = bench_now();
    for (uint64_t v = 0; v < 20000; v++) {
        checksum += (uint64_t)format_u64(v * 2654435761ULL, text) + (uint8_t)text[0];
    }
    report("format 20000 u64", bench_now() - start);

    // Memory: 1 MiB transformed byte by byte
    uint8_t *a = kmalloc(1 << 20);
    uint8_t *b = kmalloc(1 << 20);
    start = bench_now();
    for (uint64_t i = 0; i < (1 << 20); i++) {
        b[i] = (uint8_t)(a[i] ^ (uint8_t)i);
    }
    report("copy 1 MiB", bench_now() - start);
    checksum += b[12345];
    kfree(b);
    kfree(a);

    // Framebuffer: 4000 glyphs through the 8x8 font
    start = bench_now();
    for (int i = 0; i < 4000; i++) {
        draw_char((i % 100) * 8, 600, (char)('A' + i % 26), 0x00FFFFFF);
    }
    report("draw 4000 glyphs", bench_now() - start);

    kprintf("[BENCH] checksum: %lu\n", checksum);
}
