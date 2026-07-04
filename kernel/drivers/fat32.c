#include "fat32.h"

int cluster_to_lba(uint32_t cluster_number, uint32_t* lba, struct bios_parameter_block* bpb) {    
    // Calculate the first data sector
    uint32_t first_data_sector = bpb->reserved_sectors + (bpb->num_fats * bpb->fat_size_32);

    // Calculate the LBA of the given cluster
    *lba = bpb->hidden_sectors + first_data_sector + ((cluster_number - 2) * bpb->sectors_per_cluster);

    return *lba;
}

int fat32_find_file(uint8_t* dir_buf, const char* name, uint32_t* cluster, uint32_t* size) {
    // Assuming 512 bytes per sector and 32 bytes per directory entry
    for (int i = 0; i < 512 / 32; i++) {
        uint8_t* entry = dir_buf + (i * 32);
        if (entry[0] == 0x00) {
            // No more entries
            return -1;
        }
        if (entry[0] == 0xE5) {
            // Deleted entry
            continue;
        }
        if (entry[11] == 0x0F) {
            // Long file name entry
            continue;
        }

        char filename[12];
        for (int j = 0; j < 11; j++) {
            filename[j] = entry[j];
        }
        filename[11] = '\0';

        kprintf("Checking file: %s against %s\n", filename, name);
        int res = strcmp(filename, name);
        kprintf("strcmp result: %d\n", res);
        if (res == 0) {
            // High and low cluster
            *cluster = *(uint16_t*)(entry + 20) << 16 | *(uint16_t*)(entry + 26);
            *size = *(uint32_t*)(entry + 28);
            return 0; // Found
        }
    }
    return -1; // Not found
}

int fat32_read_file(uint32_t start_cluster, struct bios_parameter_block* bpb, uint8_t* out) {
    uint32_t current_cluster = start_cluster;
    uint32_t lba;
    uint32_t bytes_per_sector = bpb->bytes_per_sector;
    uint32_t sectors_per_cluster = bpb->sectors_per_cluster;
    uint8_t fat_buffer[512];

    // 0x0FFFFFF8 to 0x0FFFFFFF are reserved values indicating end of cluster chain
    while (current_cluster < 0x0FFFFFF8) {
        cluster_to_lba(current_cluster, &lba, bpb);
        ata_read_sector(0, lba, sectors_per_cluster, out);

        // Move to the next cluster in the chain
        uint32_t fat_sector_offset = (current_cluster * 4) / bytes_per_sector;
        uint32_t fat_entry_lba = bpb->hidden_sectors + bpb->reserved_sectors + fat_sector_offset;
        ata_read_sector(0, fat_entry_lba, 1, fat_buffer);
        // Mask to get the 28 bits
        current_cluster = *(uint32_t *)(fat_buffer + (current_cluster * 4) % bytes_per_sector) & 0x0FFFFFFF;

        // Move the output pointer forward by the size of the cluster
        out += bytes_per_sector * sectors_per_cluster;
    }

    // Success
    return 0;
}


// Code for me to save to test things maybe later on idfk, im just keeping it so I dont have to write it again if I need it
/* 
    for (int i = 0; i < 128; i++) {
        // 0x00 = no more entries, stop scanning
        // 0xE5 = deleted entry, skip it
        if (new_buffer[i * 32] == 0x00) {
            kprintf("No more entries found, stopping scan.\n");
            break;
        }
        if (new_buffer[i * 32] == 0xE5) {
            kprintf("Deleted entry found, skipping.\n");
            continue;
        }
        if (new_buffer[i * 32 + 11] == 0x0F) {
            kprintf("Long file name entry found, skipping.\n");
            continue;
        }
        char filename[12];
        for (int j = 0; j < 11; j++) {
            filename[j] = new_buffer[i * 32 + j];
        }
        filename[11] = '\0';
        kprintf("File: %s\n", filename);
        // We have the filename, now let's get the starting cluster and file size
        uint16_t high_cluster = *(uint16_t *)(new_buffer + i * 32 + 20);
        uint16_t low_cluster  = *(uint16_t *)(new_buffer + i * 32 + 26);
        uint32_t starting_cluster = (high_cluster << 16) | low_cluster;
        uint32_t file_size = *(uint32_t *)(new_buffer + i * 32 + 28);
        kprintf("Starting Cluster: %u, File Size: %u bytes\n", starting_cluster, file_size);
        // read a FAT entry to find the next cluster in the chain
        uint8_t file_buffer = kmalloc(file_size);
        if (fat32_read_file(bpb->root_cluster, bpb, file_buffer) != 0) {
            kprintf("Failed to read file starting at cluster %u\n", starting_cluster);
        } else {
            kprintf("Successfully read file starting at cluster %u\n", starting_cluster);
        }
    }
*/
