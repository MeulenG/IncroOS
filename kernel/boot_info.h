#ifndef __BOOT_INFO_H__
#define __BOOT_INFO_H__

#include <stdint.h>
#include "drivers/vesa.h"

// The block stage2 fills in and passes to kMain in RDI.
// Keep the layout in sync with boot/includes/BootInfo.inc.
#define BOOT_INFO_MAGIC 0x52434E49 // "INCR"
#define BOOT_INFO_VERSION 1
#define BOOT_INFO_MAX_MEMORY_ENTRIES 32

// E820 region types
#define E820_USABLE 1

struct e820_entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t acpi;
} __attribute__((packed));

struct boot_info {
    uint32_t magic;
    uint32_t version;
    uint32_t size;
    uint8_t boot_drive;
    uint8_t reserved[3];
    uint64_t kernel_address;
    uint64_t kernel_size;
    uint32_t memory_map_count;
    uint32_t memory_map_entry_size;
    struct e820_entry memory_map[BOOT_INFO_MAX_MEMORY_ENTRIES];
    struct vbe_mode_info_structure vbe_mode_info;
} __attribute__((packed));

// Set by kMain once the block has been checked
extern const struct boot_info* g_boot_info;

#endif
