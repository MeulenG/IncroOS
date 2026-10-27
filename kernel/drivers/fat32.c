#include "fat32.h"
#include "../memory/kmalloc.h"

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

// The ATA driver only does 512 byte sectors
#define SECTOR_SIZE         512
#define FAT32_EOC           0x0FFFFFFF
#define ATTR_DIRECTORY      0x10
#define ATTR_ARCHIVE        0x20
// There is no RTC driver yet, so every entry is stamped 1980-01-01 00:00 (FAT date 0 is invalid)
#define FAT_DATE_1980_01_01 0x0021

// Allocation state for the one mounted volume
static uint32_t next_free_hint = 2;
static int fsinfo_invalidated = 0;

static uint32_t fat32_cluster_count(struct bios_parameter_block* bpb) {
    uint32_t first_data_sector = bpb->reserved_sectors + (bpb->num_fats * bpb->fat_size_32);
    return (bpb->total_sectors_32 - first_data_sector) / bpb->sectors_per_cluster;
}

static uint32_t fat32_next_cluster(struct bios_parameter_block* bpb, uint32_t cluster) {
    uint8_t sector[SECTOR_SIZE];
    ata_read_sector(0, bpb->hidden_sectors + bpb->reserved_sectors + (cluster * 4) / SECTOR_SIZE, 1, sector);
    return *(uint32_t*)(sector + (cluster * 4) % SECTOR_SIZE) & 0x0FFFFFFF;
}

// Writes a FAT entry into every FAT copy, keeping the reserved top 4 bits
static int fat32_set_entry(struct bios_parameter_block* bpb, uint32_t cluster, uint32_t value) {
    uint8_t sector[SECTOR_SIZE];
    for (uint8_t fat = 0; fat < bpb->num_fats; fat++) {
        uint32_t lba = bpb->hidden_sectors + bpb->reserved_sectors + fat * bpb->fat_size_32 + (cluster * 4) / SECTOR_SIZE;
        ata_read_sector(0, lba, 1, sector);
        uint32_t* entry = (uint32_t*)(sector + (cluster * 4) % SECTOR_SIZE);
        *entry = (*entry & 0xF0000000) | (value & 0x0FFFFFFF);
        if (ata_write_sector(0, lba, 1, sector) != 0) {
            return -1;
        }
    }
    return 0;
}

// FSInfo's free cluster count is only a hint, so mark it unknown rather than keep it in sync
static int fat32_invalidate_fsinfo(struct bios_parameter_block* bpb) {
    uint8_t sector[SECTOR_SIZE];
    uint32_t lba = bpb->hidden_sectors + bpb->fs_info;
    ata_read_sector(0, lba, 1, sector);
    if (*(uint32_t*)sector != 0x41615252) {
        // No valid FSInfo sector, nothing to keep consistent
        return 0;
    }
    *(uint32_t*)(sector + 488) = 0xFFFFFFFF; // free cluster count
    *(uint32_t*)(sector + 492) = 0xFFFFFFFF; // next free cluster
    return ata_write_sector(0, lba, 1, sector);
}

// Finds a free cluster, marks it end of chain and links it after prev (0 starts a new chain)
static int fat32_alloc_cluster(struct bios_parameter_block* bpb, uint32_t prev, uint32_t* cluster) {
    uint32_t count = fat32_cluster_count(bpb);
    uint8_t sector[SECTOR_SIZE];
    uint32_t loaded_lba = 0;

    for (uint32_t i = 0; i < count; i++) {
        uint32_t candidate = 2 + (next_free_hint - 2 + i) % count;
        uint32_t lba = bpb->hidden_sectors + bpb->reserved_sectors + (candidate * 4) / SECTOR_SIZE;
        if (lba != loaded_lba) {
            ata_read_sector(0, lba, 1, sector);
            loaded_lba = lba;
        }
        if ((*(uint32_t*)(sector + (candidate * 4) % SECTOR_SIZE) & 0x0FFFFFFF) != 0) {
            continue;
        }

        if (!fsinfo_invalidated) {
            if (fat32_invalidate_fsinfo(bpb) != 0) {
                return -1;
            }
            fsinfo_invalidated = 1;
        }
        if (fat32_set_entry(bpb, candidate, FAT32_EOC) != 0) {
            return -1;
        }
        if (prev != 0 && fat32_set_entry(bpb, prev, candidate) != 0) {
            return -1;
        }
        next_free_hint = candidate + 1;
        *cluster = candidate;
        return 0;
    }
    return -1; // Volume is full
}

static void fat32_fill_entry(uint8_t* entry, const char* name, uint8_t attr, uint32_t cluster, uint32_t size) {
    for (int i = 0; i < 32; i++) {
        entry[i] = 0;
    }
    for (int i = 0; i < 11; i++) {
        entry[i] = name[i];
    }
    entry[11] = attr;
    *(uint16_t*)(entry + 16) = FAT_DATE_1980_01_01; // creation date
    *(uint16_t*)(entry + 18) = FAT_DATE_1980_01_01; // last access date
    *(uint16_t*)(entry + 20) = (uint16_t)(cluster >> 16);
    *(uint16_t*)(entry + 24) = FAT_DATE_1980_01_01; // write date
    *(uint16_t*)(entry + 26) = (uint16_t)(cluster & 0xFFFF);
    *(uint32_t*)(entry + 28) = size;
}

// Scans a directory chain for a free entry, fails if the name is already taken.
// found is 0 when every entry is in use, cluster is then the last cluster of the chain.
static int fat32_find_free_entry(struct bios_parameter_block* bpb, uint32_t dir_cluster, const char* name,
                                 uint8_t* buffer, int* found, uint32_t* lba_out, uint32_t* offset_out, uint32_t* cluster) {
    uint32_t cluster_bytes = SECTOR_SIZE * bpb->sectors_per_cluster;
    *found = 0;
    *cluster = dir_cluster;

    while (1) {
        uint32_t lba;
        cluster_to_lba(*cluster, &lba, bpb);
        ata_read_sector(0, lba, bpb->sectors_per_cluster, buffer);

        for (uint32_t offset = 0; offset < cluster_bytes; offset += 32) {
            uint8_t* entry = buffer + offset;
            if (entry[0] == 0x00 || entry[0] == 0xE5) {
                if (!*found) {
                    *found = 1;
                    *lba_out = lba + offset / SECTOR_SIZE;
                    *offset_out = offset % SECTOR_SIZE;
                }
                if (entry[0] == 0x00) {
                    // End of directory, no entries are in use after this one
                    return 0;
                }
                continue;
            }
            if (entry[11] == 0x0F) {
                // Long file name entry
                continue;
            }

            int same = 1;
            for (int i = 0; i < 11; i++) {
                if (entry[i] != (uint8_t)name[i]) {
                    same = 0;
                    break;
                }
            }
            if (same) {
                return -1;
            }
        }

        uint32_t next = fat32_next_cluster(bpb, *cluster);
        if (next >= 0x0FFFFFF8) {
            return 0;
        }
        *cluster = next;
    }
}

static int fat32_add_entry(struct bios_parameter_block* bpb, uint32_t dir_cluster, const char* name, uint8_t attr,
                           uint32_t first_cluster, struct fat32_file* file) {
    for (int i = 0; i < 11; i++) {
        if (name[i] == '\0') {
            return -1;
        }
    }
    if (name[0] == ' ' || (uint8_t)name[0] == 0xE5) {
        return -1;
    }

    uint32_t cluster_bytes = SECTOR_SIZE * bpb->sectors_per_cluster;
    uint8_t* buffer = kmalloc(cluster_bytes);
    if (buffer == NULL) {
        return -1;
    }

    int found;
    uint32_t lba, offset, last_cluster;
    int status = fat32_find_free_entry(bpb, dir_cluster, name, buffer, &found, &lba, &offset, &last_cluster);

    if (status == 0 && !found) {
        // Directory is full, extend it with a zeroed cluster
        uint32_t new_cluster;
        status = fat32_alloc_cluster(bpb, last_cluster, &new_cluster);
        if (status == 0) {
            for (uint32_t i = 0; i < cluster_bytes; i++) {
                buffer[i] = 0;
            }
            cluster_to_lba(new_cluster, &lba, bpb);
            offset = 0;
            status = ata_write_sector(0, lba, bpb->sectors_per_cluster, buffer);
        }
    }

    if (status == 0) {
        uint8_t sector[SECTOR_SIZE];
        ata_read_sector(0, lba, 1, sector);
        fat32_fill_entry(sector + offset, name, attr, first_cluster, 0);
        status = ata_write_sector(0, lba, 1, sector);
    }

    if (status == 0) {
        file->entry_lba = lba;
        file->entry_offset = offset;
        file->first_cluster = first_cluster;
        file->last_cluster = first_cluster;
        file->size = 0;
    }
    kfree(buffer);
    return status;
}

// Writes the first cluster and size back into the file's directory entry
static int fat32_update_entry(struct fat32_file* file) {
    uint8_t sector[SECTOR_SIZE];
    ata_read_sector(0, file->entry_lba, 1, sector);
    uint8_t* entry = sector + file->entry_offset;
    *(uint16_t*)(entry + 20) = (uint16_t)(file->first_cluster >> 16);
    *(uint16_t*)(entry + 26) = (uint16_t)(file->first_cluster & 0xFFFF);
    *(uint32_t*)(entry + 28) = file->size;
    return ata_write_sector(0, file->entry_lba, 1, sector);
}

int fat32_create_file(struct bios_parameter_block* bpb, uint32_t dir_cluster, const char* name, struct fat32_file* file) {
    // Empty files have no clusters
    return fat32_add_entry(bpb, dir_cluster, name, ATTR_ARCHIVE, 0, file);
}

// Appends data to the end of the file
int fat32_write_file(struct bios_parameter_block* bpb, struct fat32_file* file, const uint8_t* data, uint32_t length) {
    if (length > 0xFFFFFFFF - file->size) {
        return -1; // FAT32 files are at most 4GB
    }

    uint32_t cluster_bytes = SECTOR_SIZE * bpb->sectors_per_cluster;
    uint8_t* buffer = kmalloc(cluster_bytes);
    if (buffer == NULL) {
        return -1;
    }
    // Sectors are written whole, so clear the buffer to keep old heap contents off the disk
    for (uint32_t i = 0; i < cluster_bytes; i++) {
        buffer[i] = 0;
    }

    int status = 0;
    while (length > 0) {
        uint32_t offset = file->size % cluster_bytes;
        if (file->first_cluster == 0) {
            status = fat32_alloc_cluster(bpb, 0, &file->first_cluster);
            file->last_cluster = file->first_cluster;
        } else if (offset == 0) {
            // The last cluster is full
            status = fat32_alloc_cluster(bpb, file->last_cluster, &file->last_cluster);
        }
        if (status != 0) {
            break;
        }

        uint32_t chunk = cluster_bytes - offset;
        if (chunk > length) {
            chunk = length;
        }
        uint32_t first_sector = offset / SECTOR_SIZE;
        uint32_t end_sector = (offset + chunk + SECTOR_SIZE - 1) / SECTOR_SIZE;
        uint32_t lba;
        cluster_to_lba(file->last_cluster, &lba, bpb);

        // Keep the bytes already written to a partly filled sector
        if (offset % SECTOR_SIZE != 0) {
            ata_read_sector(0, lba + first_sector, 1, buffer + first_sector * SECTOR_SIZE);
        }
        for (uint32_t i = 0; i < chunk; i++) {
            buffer[offset + i] = data[i];
        }
        status = ata_write_sector(0, lba + first_sector, end_sector - first_sector, buffer + first_sector * SECTOR_SIZE);
        if (status != 0) {
            break;
        }

        file->size += chunk;
        data += chunk;
        length -= chunk;
    }

    if (status == 0) {
        status = fat32_update_entry(file);
    }
    kfree(buffer);
    return status;
}

int fat32_mkdir(struct bios_parameter_block* bpb, uint32_t parent_cluster, const char* name, uint32_t* cluster) {
    uint32_t cluster_bytes = SECTOR_SIZE * bpb->sectors_per_cluster;
    uint8_t* buffer = kmalloc(cluster_bytes);
    if (buffer == NULL) {
        return -1;
    }

    uint32_t dir;
    int status = fat32_alloc_cluster(bpb, 0, &dir);
    if (status == 0) {
        for (uint32_t i = 0; i < cluster_bytes; i++) {
            buffer[i] = 0;
        }
        // ".." of a directory in the root points at cluster 0, not the root cluster
        fat32_fill_entry(buffer, ".          ", ATTR_DIRECTORY, dir, 0);
        fat32_fill_entry(buffer + 32, "..         ", ATTR_DIRECTORY,
                         parent_cluster == bpb->root_cluster ? 0 : parent_cluster, 0);

        uint32_t lba;
        cluster_to_lba(dir, &lba, bpb);
        status = ata_write_sector(0, lba, bpb->sectors_per_cluster, buffer);
    }

    if (status == 0) {
        struct fat32_file entry;
        status = fat32_add_entry(bpb, parent_cluster, name, ATTR_DIRECTORY, dir, &entry);
        if (status != 0) {
            // Release the cluster again, the name was taken or the parent could not grow
            fat32_set_entry(bpb, dir, 0);
        }
    }

    if (status == 0) {
        *cluster = dir;
    }
    kfree(buffer);
    return status;
}

int validate_fat32_write(struct bios_parameter_block* bpb) {
    uint32_t dir;
    if (fat32_mkdir(bpb, bpb->root_cluster, "INCROTST   ", &dir) != 0) {
        kprintf("FAT32 write validation failed: could not create the test directory\n");
        return -1;
    }
    uint32_t duplicate;
    if (fat32_mkdir(bpb, bpb->root_cluster, "INCROTST   ", &duplicate) == 0) {
        kprintf("FAT32 write validation failed: a duplicate name was accepted\n");
        return -1;
    }

    // Three appends: a partial sector, a whole cluster starting mid sector, then a partial sector in a new cluster
    uint32_t cluster_bytes = SECTOR_SIZE * bpb->sectors_per_cluster;
    uint32_t lengths[3] = {100, cluster_bytes, 700};
    uint32_t total = lengths[0] + lengths[1] + lengths[2];
    uint32_t clusters = (total + cluster_bytes - 1) / cluster_bytes;

    uint8_t* data = kmalloc(total);
    // fat32_read_file writes whole clusters
    uint8_t* readback = kmalloc(clusters * cluster_bytes);
    if (data == NULL || readback == NULL) {
        kprintf("FAT32 write validation failed: out of memory\n");
        return -1;
    }
    for (uint32_t i = 0; i < total; i++) {
        data[i] = (uint8_t)(i * 7 + 3);
    }

    struct fat32_file file;
    if (fat32_create_file(bpb, dir, "WRITE   TST", &file) != 0) {
        kprintf("FAT32 write validation failed: could not create the test file\n");
        return -1;
    }
    uint32_t written = 0;
    for (int i = 0; i < 3; i++) {
        if (fat32_write_file(bpb, &file, data + written, lengths[i]) != 0) {
            kprintf("FAT32 write validation failed: write %d failed\n", i);
            return -1;
        }
        written += lengths[i];
    }
    if (file.size != total) {
        kprintf("FAT32 write validation failed: size is %u, expected %u\n", file.size, total);
        return -1;
    }

    // Read it back through the existing read path
    fat32_read_file(file.first_cluster, bpb, readback);
    for (uint32_t i = 0; i < total; i++) {
        if (readback[i] != data[i]) {
            kprintf("FAT32 write validation failed: byte %u differs after read back\n", i);
            return -1;
        }
    }

    // The directory entry on disk has to carry the first cluster and the final size
    uint8_t dir_sector[SECTOR_SIZE];
    uint32_t dir_lba, found_cluster, found_size;
    cluster_to_lba(dir, &dir_lba, bpb);
    ata_read_sector(0, dir_lba, 1, dir_sector);
    if (fat32_find_file(dir_sector, "WRITE   TST", &found_cluster, &found_size) != 0
        || found_cluster != file.first_cluster || found_size != total) {
        kprintf("FAT32 write validation failed: directory entry does not match the file\n");
        return -1;
    }

    // Fill the directory past one cluster so it has to grow, "." ".." and WRITE.TST are already there
    uint32_t entries_per_cluster = cluster_bytes / 32;
    char name[12] = "FILL000 TMP";
    for (uint32_t i = 0; i < entries_per_cluster - 2; i++) {
        name[4] = '0' + (i / 100) % 10;
        name[5] = '0' + (i / 10) % 10;
        name[6] = '0' + i % 10;
        struct fat32_file fill;
        if (fat32_create_file(bpb, dir, name, &fill) != 0) {
            kprintf("FAT32 write validation failed: could not create %s\n", name);
            return -1;
        }
    }
    if (fat32_next_cluster(bpb, dir) >= 0x0FFFFFF8) {
        kprintf("FAT32 write validation failed: the directory did not grow\n");
        return -1;
    }

    kfree(readback);
    kfree(data);
    kprintf("FAT32 write validation passed!\n");
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
