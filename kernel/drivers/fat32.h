#ifndef __FAT_32_H__
#define __FAT_32_H__

#include <stdint.h>
#include "../../libs/libkernel/string.h"

struct bios_parameter_block {
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;

    // FAT32 specific fields
    uint32_t fat_size_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot_sector;
} __attribute__((packed));

int cluster_to_lba(uint32_t cluster_number, uint32_t* lba, struct bios_parameter_block* bpb);
int fat32_find_file(uint8_t* dir_buf, const char* name, uint32_t* cluster, uint32_t* size);
int fat32_read_file(uint32_t start_cluster, struct bios_parameter_block* bpb, uint8_t* out);

#endif
