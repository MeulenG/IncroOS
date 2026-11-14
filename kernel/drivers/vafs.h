#ifndef __VAFS_H__
#define __VAFS_H__

#include <stdint.h>
#include "fat32.h"

// Read-only access to a VaFS pack (what bake builds) that is loaded into memory.
// The on-disk format is libvafs' (External/bake/libs/vafs/libvafs), the reference implementation.

#define VAFS_NODE_FILE      1
#define VAFS_NODE_DIRECTORY 2
#define VAFS_NODE_SYMLINK   3
#define VAFS_MAX_NAME       255

// A spot in a stream: which decoded block, and where in it
struct vafs_position {
    uint32_t index;
    uint32_t offset;
} __attribute__((packed));

struct vafs_node {
    uint16_t type;
    uint32_t size;        // files: length in bytes
    uint32_t permissions;
    // files: where the content starts in the data stream,
    // directories: where their entry list starts in the descriptor stream
    struct vafs_position position;
    char name[VAFS_MAX_NAME + 1];
    char target[VAFS_MAX_NAME + 1]; // symlinks: what they point to
};

// Walks the entries of one directory
struct vafs_dir {
    struct vafs_position next;
    uint32_t remaining;
};

struct vafs;

// The image has to stay around until vafs_unmount
int vafs_mount(const uint8_t* image, uint32_t size, struct vafs** vafs);
void vafs_unmount(struct vafs* vafs);

// Paths are absolute, like "/usr/bin/hello-world". Symlinks are not followed.
int vafs_lookup(struct vafs* vafs, const char* path, struct vafs_node* node);
int vafs_opendir(struct vafs* vafs, const struct vafs_node* dir, struct vafs_dir* it);
// 1 when an entry was read, 0 at the end, -1 on error
int vafs_readdir(struct vafs* vafs, struct vafs_dir* it, struct vafs_node* entry);
// Bytes read (0 at or past the end of the file), -1 on error
int vafs_read(struct vafs* vafs, const struct vafs_node* file, uint32_t offset, void* buffer, uint32_t length);

int validate_vafs(struct bios_parameter_block* bpb);

#endif
