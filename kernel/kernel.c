#include "drivers/serial.h"
#include "output/vga_terminal.h"
#include "output/boot_service.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "memory/kmalloc.h"
#include "../libs/libkernel/print.h"
#include "../libs/libkernel/string.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/pic.h"
#include "drivers/vesa.h"
#include "output/vesa_terminal.h"
#include "drivers/ata.h"
#include "drivers/fat32.h"

static void uint64_to_string(uint64_t value, char* buffer) {
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    char temp[32];
    int i = 0;
    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }

    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';
}

void kMain(void) {
    serial_init();

    serial_writestring("\n\n=== IncroOS Kernel Starting ===\n");

    terminal_initialize();

    // call all services using the boot service framework
    struct boot_service services[] = {
        {"GDT Initialization\n", init_gdt, TRUE},
        {"IDT Initialization\n", fill_idt_slots, TRUE},
        {"PIC Remapping\n", (int (*)())pic_init, TRUE},
        {"PMM Initialization\n", (int (*)())pmm_init, TRUE},
        {"VMM Initialization\n", (int (*)())vmm_init, TRUE},
        {"KMALLOC Initialization\n", (int (*)())kmalloc_init, TRUE},
        {"VESA Initialization\n", (int (*)())vesa_terminal_initialize, TRUE},
        {"ATA Initialization\n", (int (*)())ata_initialize, TRUE}
    };

    
    run_boot_services(services, sizeof(services) / sizeof(services[0]));
    vesa_terminal_writestring("All boot services completed successfully!\n", VESA_COLOR_LIGHT_GREEN);

    // lets test a read
    uint8_t buffer[512];
    ata_read_sector(0, 0, 1, buffer);
    kprintf("Check buffer: %d\n", buffer[510]);
    kprintf("Check buffer: %d\n", buffer[511]);

    uint32_t partition_start = *(uint32_t *)(buffer + 454);
    kprintf("Partition Start: %u\n", partition_start);

    ata_read_sector(0, partition_start, 1, buffer);

    struct bios_parameter_block *bpb = (struct bios_parameter_block *)(buffer + 11);

    // FAT start sector is calculated as follows:
    uint32_t fat_start_sector = bpb->reserved_sectors;
    uint32_t data_start_sector  = bpb->reserved_sectors + (bpb->num_fats * bpb->fat_size_32);
    kprintf("FAT Start Sector: %u\n", fat_start_sector);
    kprintf("Data Start Sector: %u\n", data_start_sector);
    // print root cluster
    kprintf("Root Cluster: %u\n", bpb->root_cluster);

    uint32_t root_lba = cluster_to_lba(bpb->root_cluster, &partition_start, bpb);
    kprintf("Root Directory LBA: %u\n", root_lba);


    // new buffer
    uint8_t new_buffer[4096];
    ata_read_sector(0, root_lba, 8, new_buffer);

    uint32_t found_cluster;
    uint32_t file_size;
    if(fat32_find_file(new_buffer, "KERNEL  BIN", &found_cluster, &file_size) == 0) {
        kprintf("Found KERNEL.BIN at cluster %u\n", found_cluster);
    } else {
        kprintf("KERNEL.BIN not found\n");
    }
    // read a FAT entry to find the next cluster in the chain
    uint8_t* file_buffer = kmalloc(file_size);
    if (fat32_read_file(found_cluster, bpb, file_buffer) != 0) {
        kprintf("Failed to read file starting at cluster %u\n", found_cluster);
    } else {
        kprintf("Successfully read file starting at cluster %u\n", found_cluster);
    }
    
    // trigger divide by 0
    // volatile int x = 1 / 0;

    // Needs to fix this and add it into the boot service at some point where it self-tests the services as a part of the bootup
    /*
    

    char buffer[32];
    uint64_to_string(pmm_get_total_pages(), buffer);
    serial_writestring("[PMM] Total pages: ");
    serial_writestring(buffer);
    serial_writestring("\n");

    uint64_to_string(pmm_get_free_pages(), buffer);
    serial_writestring("[PMM] Free pages: ");
    serial_writestring(buffer);
    serial_writestring("\n");

    uint64_to_string(pmm_get_used_pages(), buffer);
    serial_writestring("[PMM] Used pages: ");
    serial_writestring(buffer);
    serial_writestring("\n");

    serial_writestring("\n[INIT] Memory Subsystem Initialized Successfully\n");

    serial_writestring("\n[TEST] Testing Memory Allocation...\n");

    void* ptr1 = kmalloc(64);
    if (ptr1) {
        serial_writestring("[TEST] kmalloc(64) succeeded\n");
        kfree(ptr1);
        serial_writestring("[TEST] kfree(64) succeeded\n");
    } else {
        serial_writestring("[TEST] kmalloc(64) failed\n");
    }

    void* ptr2 = kmalloc(1024);
    if (ptr2) {
        serial_writestring("[TEST] kmalloc(1024) succeeded\n");
        kfree(ptr2);
        serial_writestring("[TEST] kfree(1024) succeeded\n");
    } else {
        serial_writestring("[TEST] kmalloc(1024) failed\n");
    }

    uint64_t page = pmm_alloc_page();
    if (page) {
        serial_writestring("[TEST] pmm_alloc_page() succeeded, page at: 0x");
        for (int i = 60; i >= 0; i -= 4) {
            uint8_t nibble = (page >> i) & 0xF;
            char hex = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
            char hex_str[2] = {hex, '\0'};
            serial_writestring(hex_str);
        }
        serial_writestring("\n");
        pmm_free_page(page);
        serial_writestring("[TEST] pmm_free_page() succeeded\n");
    } else {
        serial_writestring("[TEST] pmm_alloc_page() failed\n");
    }

    uint64_to_string(kmalloc_get_used(), buffer);
    serial_writestring("\n[HEAP] Used memory: ");
    serial_writestring(buffer);
    serial_writestring(" bytes\n");

    uint64_to_string(kmalloc_get_free(), buffer);
    serial_writestring("[HEAP] Free memory: ");
    serial_writestring(buffer);
    serial_writestring(" bytes\n");

    serial_writestring("\n===========================================\n");
    serial_writestring("  Memory Manager Tests Complete\n");
    serial_writestring("===========================================\n");

    terminal_writestring("Memory Manager Initialized!\n");
    kprintf("%d %d %d\n", 0, -1, 1234);
    kprintf("hello %s, count=%d, addr=%p\n", "world", 42, (void*)0xDEAD);
    */

    while (1) {
        __asm__ volatile("hlt");
    }
}
