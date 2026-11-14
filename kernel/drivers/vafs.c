#include "vafs.h"
#include "../memory/kmalloc.h"
#include "../../libs/zstd/zstd.h"

#define VAFS_MAGIC          0x3144524D
#define VAFS_VERSION        0x00010000
#define VAFS_STREAM_MAGIC   0x314D5356
#define VAFS_MAX_BLOCK_SIZE (1024 * 1024)
// The largest descriptor: a symlink with a full length name and target
#define VAFS_MAX_DESCRIPTOR (8 + 2 * VAFS_MAX_NAME)

struct vafs_header {
    uint32_t magic;
    uint32_t version;
    uint32_t architecture;
    uint16_t feature_count;
    uint16_t reserved;
    uint32_t attributes;
    uint32_t descriptor_offset;
    uint32_t data_offset;
    struct vafs_position root; // the root directory's entry list, there is no descriptor for it
} __attribute__((packed));

struct vafs_feature_header {
    uint8_t guid[16];
    uint32_t length; // including this header
} __attribute__((packed));

struct vafs_stream_header {
    uint32_t magic;
    uint32_t block_size;           // decoded size of every block but the last
    uint32_t block_headers_offset; // from the start of the stream
    uint32_t block_headers_count;
} __attribute__((packed));

struct vafs_block_header {
    uint32_t length_on_disk;
    uint32_t offset; // from the start of the stream, header included
    uint32_t crc;    // of the decoded block
    uint16_t flags;
    uint16_t reserved;
} __attribute__((packed));

struct vafs_descriptor {
    uint16_t type;
    uint16_t length; // whole descriptor, name (and target) included
} __attribute__((packed));

struct vafs_file_descriptor {
    struct vafs_descriptor base;
    struct vafs_position data;
    uint32_t file_length;
    uint32_t permissions;
} __attribute__((packed));

struct vafs_directory_descriptor {
    struct vafs_descriptor base;
    struct vafs_position descriptor;
    uint32_t permissions;
} __attribute__((packed));

struct vafs_symlink_descriptor {
    struct vafs_descriptor base;
    uint16_t name_length;
    uint16_t target_length;
} __attribute__((packed));

struct vafs_stream {
    uint32_t start; // offset of the stream in the image
    uint32_t block_size;
    uint32_t block_count;
    const struct vafs_block_header* blocks;
    int32_t cached_index; // block decoded into cache, -1 for none
    uint32_t cached_length;
    uint8_t* cache;
};

struct vafs {
    const uint8_t* image;
    uint32_t size;
    int zstd;
    ZSTD_DCtx* dctx;
    struct vafs_position root;
    struct vafs_stream descriptors;
    struct vafs_stream data;
};

// The filter feature. bake installs it without a payload, and then it always means zstd.
static const uint8_t vafs_filter_guid[16] = {
    0x91, 0x5D, 0xC2, 0x99, 0x99, 0xFA, 0x71, 0x4A,
    0x9C, 0xB5, 0x96, 0x1A, 0xA9, 0x3D, 0xDF, 0xBB
};

static uint32_t vafs_crc_table[256];
static int vafs_crc_ready = 0;

// libvafs' crc.c: CRC-32, polynomial 0x04C11DB7, not reflected, starts at 0xFFFFFFFF, inverted
static uint32_t vafs_crc(const uint8_t* data, uint32_t length) {
    if (!vafs_crc_ready) {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t value = i << 24;
            for (int bit = 0; bit < 8; bit++) {
                value = (value & 0x80000000) ? (value << 1) ^ 0x04C11DB7 : value << 1;
            }
            vafs_crc_table[i] = value;
        }
        vafs_crc_ready = 1;
    }

    uint32_t value = 0xFFFFFFFF;
    for (uint32_t i = 0; i < length; i++) {
        value = (value << 8) ^ vafs_crc_table[((value >> 24) ^ data[i]) & 0xFF];
    }
    return ~value;
}

// Is [offset, offset + length) inside the image
static int vafs_in_image(struct vafs* vafs, uint32_t offset, uint32_t length) {
    return offset <= vafs->size && length <= vafs->size - offset;
}

static int vafs_stream_open(struct vafs* vafs, uint32_t start, struct vafs_stream* stream) {
    if (!vafs_in_image(vafs, start, sizeof(struct vafs_stream_header))) {
        return -1;
    }
    const struct vafs_stream_header* header = (const struct vafs_stream_header*)(vafs->image + start);
    if (header->magic != VAFS_STREAM_MAGIC || header->block_size == 0 || header->block_size > VAFS_MAX_BLOCK_SIZE
        || header->block_headers_offset > vafs->size - start) {
        return -1;
    }
    uint32_t table = start + header->block_headers_offset;
    if (header->block_headers_count > (vafs->size - table) / sizeof(struct vafs_block_header)) {
        return -1;
    }

    stream->start = start;
    stream->block_size = header->block_size;
    stream->block_count = header->block_headers_count;
    stream->blocks = (const struct vafs_block_header*)(vafs->image + table);
    stream->cached_index = -1;
    stream->cached_length = 0;
    stream->cache = kmalloc(header->block_size);
    return stream->cache != NULL ? 0 : -1;
}

static int vafs_load_block(struct vafs* vafs, struct vafs_stream* stream, uint32_t index) {
    if (stream->cached_index == (int32_t)index) {
        return 0;
    }
    if (index >= stream->block_count) {
        return -1;
    }
    const struct vafs_block_header* block = &stream->blocks[index];
    if (block->offset > vafs->size - stream->start
        || !vafs_in_image(vafs, stream->start + block->offset, block->length_on_disk)) {
        return -1;
    }
    const uint8_t* source = vafs->image + stream->start + block->offset;

    // the cache gets overwritten now, so it holds no valid block until this succeeds
    stream->cached_index = -1;
    uint32_t length;
    if (vafs->zstd) {
        size_t decoded = ZSTD_decompressDCtx(vafs->dctx, stream->cache, stream->block_size, source, block->length_on_disk);
        if (ZSTD_isError(decoded)) {
            return -1;
        }
        length = (uint32_t)decoded;
    } else {
        if (block->length_on_disk > stream->block_size) {
            return -1;
        }
        memcpy(stream->cache, source, block->length_on_disk);
        length = block->length_on_disk;
    }

    // bake's zstd frames carry no checksum of their own, the block CRC is what catches a damaged pack
    if (vafs_crc(stream->cache, length) != block->crc) {
        return -1;
    }
    stream->cached_index = (int32_t)index;
    stream->cached_length = length;
    return 0;
}

// Reads length bytes from position on, across block boundaries, and moves position past them
static int vafs_stream_read(struct vafs* vafs, struct vafs_stream* stream, struct vafs_position* position,
                            void* buffer, uint32_t length) {
    uint8_t* out = buffer;
    while (length > 0) {
        if (vafs_load_block(vafs, stream, position->index) != 0) {
            return -1;
        }
        if (position->offset >= stream->cached_length) {
            // past the end of this block, the read carries on in the next one
            position->index++;
            position->offset = 0;
            continue;
        }

        uint32_t chunk = stream->cached_length - position->offset;
        if (chunk > length) {
            chunk = length;
        }
        memcpy(out, stream->cache + position->offset, chunk);
        out += chunk;
        length -= chunk;
        position->offset += chunk;
    }
    return 0;
}

int vafs_mount(const uint8_t* image, uint32_t size, struct vafs** vafs_out) {
    if (image == NULL || size < sizeof(struct vafs_header)) {
        return -1;
    }
    const struct vafs_header* header = (const struct vafs_header*)image;
    if (header->magic != VAFS_MAGIC || header->version != VAFS_VERSION) {
        return -1;
    }

    struct vafs* vafs = kmalloc(sizeof(struct vafs));
    if (vafs == NULL) {
        return -1;
    }
    memset(vafs, 0, sizeof(struct vafs));
    vafs->image = image;
    vafs->size = size;
    vafs->root = header->root;

    int status = 0;
    uint32_t offset = sizeof(struct vafs_header);
    for (uint16_t i = 0; i < header->feature_count && status == 0; i++) {
        const struct vafs_feature_header* feature = (const struct vafs_feature_header*)(image + offset);
        if (!vafs_in_image(vafs, offset, sizeof(struct vafs_feature_header))
            || feature->length < sizeof(struct vafs_feature_header) || !vafs_in_image(vafs, offset, feature->length)) {
            status = -1;
            break;
        }
        int filter = 1;
        for (int j = 0; j < 16; j++) {
            if (feature->guid[j] != vafs_filter_guid[j]) {
                filter = 0;
            }
        }
        if (filter) {
            // libvafs' own tools store a filter type (aplib or brieflz) after the header, those are not supported
            if (feature->length != sizeof(struct vafs_feature_header)) {
                status = -1;
            }
            vafs->zstd = 1;
        }
        offset += feature->length;
    }

    if (status == 0 && vafs->zstd) {
        vafs->dctx = ZSTD_createDCtx();
        if (vafs->dctx == NULL) {
            status = -1;
        }
    }
    if (status == 0) {
        status = vafs_stream_open(vafs, header->descriptor_offset, &vafs->descriptors);
    }
    if (status == 0) {
        status = vafs_stream_open(vafs, header->data_offset, &vafs->data);
    }
    if (status != 0) {
        vafs_unmount(vafs);
        return -1;
    }
    *vafs_out = vafs;
    return 0;
}

void vafs_unmount(struct vafs* vafs) {
    if (vafs == NULL) {
        return;
    }
    kfree(vafs->descriptors.cache);
    kfree(vafs->data.cache);
    ZSTD_freeDCtx(vafs->dctx);
    kfree(vafs);
}

// Reads the descriptor at position into entry and moves position past it
static int vafs_read_descriptor(struct vafs* vafs, struct vafs_position* position, struct vafs_node* entry) {
    uint8_t buffer[VAFS_MAX_DESCRIPTOR];
    struct vafs_descriptor base;
    struct vafs_position peek = *position;
    if (vafs_stream_read(vafs, &vafs->descriptors, &peek, &base, sizeof(base)) != 0
        || base.length < sizeof(base) || base.length > sizeof(buffer)
        || vafs_stream_read(vafs, &vafs->descriptors, position, buffer, base.length) != 0) {
        return -1;
    }

    memset(entry, 0, sizeof(struct vafs_node));
    entry->type = base.type;
    const uint8_t* name;
    uint32_t name_length;
    if (base.type == VAFS_NODE_FILE) {
        const struct vafs_file_descriptor* file = (const struct vafs_file_descriptor*)buffer;
        if (base.length < sizeof(*file)) {
            return -1;
        }
        entry->position = file->data;
        entry->size = file->file_length;
        entry->permissions = file->permissions;
        name = buffer + sizeof(*file);
        name_length = base.length - sizeof(*file);
    } else if (base.type == VAFS_NODE_DIRECTORY) {
        const struct vafs_directory_descriptor* dir = (const struct vafs_directory_descriptor*)buffer;
        if (base.length < sizeof(*dir)) {
            return -1;
        }
        entry->position = dir->descriptor;
        entry->permissions = dir->permissions;
        name = buffer + sizeof(*dir);
        name_length = base.length - sizeof(*dir);
    } else if (base.type == VAFS_NODE_SYMLINK) {
        const struct vafs_symlink_descriptor* link = (const struct vafs_symlink_descriptor*)buffer;
        if (base.length < sizeof(*link) || link->target_length > VAFS_MAX_NAME
            || sizeof(*link) + link->name_length + link->target_length > base.length) {
            return -1;
        }
        name = buffer + sizeof(*link);
        name_length = link->name_length;
        memcpy(entry->target, name + name_length, link->target_length);
    } else {
        return -1;
    }

    // names are not null-terminated on disk, entry is zeroed so the copy ends up terminated
    if (name_length > VAFS_MAX_NAME) {
        return -1;
    }
    memcpy(entry->name, name, name_length);
    return 0;
}

int vafs_opendir(struct vafs* vafs, const struct vafs_node* dir, struct vafs_dir* it) {
    if (dir->type != VAFS_NODE_DIRECTORY) {
        return -1;
    }
    // an entry list is a count followed by that many descriptors
    it->next = dir->position;
    return vafs_stream_read(vafs, &vafs->descriptors, &it->next, &it->remaining, sizeof(it->remaining));
}

int vafs_readdir(struct vafs* vafs, struct vafs_dir* it, struct vafs_node* entry) {
    if (it->remaining == 0) {
        return 0;
    }
    if (vafs_read_descriptor(vafs, &it->next, entry) != 0) {
        return -1;
    }
    it->remaining--;
    return 1;
}

int vafs_lookup(struct vafs* vafs, const char* path, struct vafs_node* node) {
    if (path[0] != '/') {
        return -1;
    }
    memset(node, 0, sizeof(struct vafs_node));
    node->type = VAFS_NODE_DIRECTORY;
    node->position = vafs->root;

    const char* component = path;
    while (1) {
        while (*component == '/') {
            component++;
        }
        if (*component == '\0') {
            return 0;
        }
        uint32_t length = 0;
        while (component[length] != '\0' && component[length] != '/') {
            length++;
        }

        struct vafs_dir it;
        if (vafs_opendir(vafs, node, &it) != 0) {
            return -1; // not a directory, or damaged
        }
        struct vafs_node entry;
        int status;
        int found = 0;
        while (!found && (status = vafs_readdir(vafs, &it, &entry)) == 1) {
            found = 1;
            for (uint32_t i = 0; i <= length; i++) {
                char expected = i < length ? component[i] : '\0';
                if (entry.name[i] != expected) {
                    found = 0;
                    break;
                }
            }
        }
        if (!found) {
            return -1;
        }
        memcpy(node, &entry, sizeof(struct vafs_node));
        component += length;
    }
}

int vafs_read(struct vafs* vafs, const struct vafs_node* file, uint32_t offset, void* buffer, uint32_t length) {
    if (file->type != VAFS_NODE_FILE) {
        return -1;
    }
    if (offset >= file->size) {
        return 0;
    }
    if (length > file->size - offset) {
        length = file->size - offset;
    }

    // every data block but the last decodes to block_size bytes, so the offset maps straight to a block
    uint64_t start = (uint64_t)file->position.offset + offset;
    struct vafs_position position;
    position.index = file->position.index + (uint32_t)(start / vafs->data.block_size);
    position.offset = (uint32_t)(start % vafs->data.block_size);
    if (vafs_stream_read(vafs, &vafs->data, &position, buffer, length) != 0) {
        return -1;
    }
    return (int)length;
}

// Reads a file from the root directory of the boot partition into a new buffer
static uint8_t* vafs_load_file(struct bios_parameter_block* bpb, const char* name, uint32_t* size) {
    uint8_t dir_sector[512];
    uint32_t root_lba, cluster;
    cluster_to_lba(bpb->root_cluster, &root_lba, bpb);
    ata_read_sector(0, root_lba, 1, dir_sector);
    if (fat32_find_file(dir_sector, name, &cluster, size) != 0 || *size == 0) {
        return NULL;
    }
    // fat32_read_file writes whole clusters
    uint32_t cluster_bytes = bpb->bytes_per_sector * bpb->sectors_per_cluster;
    uint8_t* buffer = kmalloc(((*size + cluster_bytes - 1) / cluster_bytes) * cluster_bytes);
    if (buffer != NULL) {
        fat32_read_file(cluster, bpb, buffer);
    }
    return buffer;
}

// HELLO.PAK is tests/vafs/hello-world.pack, built by bake. The expected values come from
// tests/vafs/vafs_ls.py.
static int vafs_check_bake_pack(struct vafs* vafs) {
    struct vafs_node root, entry;
    struct vafs_dir it;
    if (vafs_lookup(vafs, "/", &root) != 0 || vafs_opendir(vafs, &root, &it) != 0
        || vafs_readdir(vafs, &it, &entry) != 1 || entry.type != VAFS_NODE_DIRECTORY
        || entry.name[0] != 'u' || entry.name[1] != 's' || entry.name[2] != 'r' || entry.name[3] != '\0'
        || vafs_readdir(vafs, &it, &entry) != 0) {
        kprintf("VAFS validation failed: HELLO.PAK root should hold just usr/\n");
        return -1;
    }

    struct vafs_node file;
    if (vafs_lookup(vafs, "/usr/bin/hello-world", &file) != 0 || file.type != VAFS_NODE_FILE || file.size != 15960) {
        kprintf("VAFS validation failed: HELLO.PAK /usr/bin/hello-world missing or the wrong size\n");
        return -1;
    }
    uint8_t* content = kmalloc(file.size);
    if (content == NULL || vafs_read(vafs, &file, 0, content, file.size) != (int)file.size) {
        kprintf("VAFS validation failed: could not read HELLO.PAK /usr/bin/hello-world\n");
        return -1;
    }
    uint32_t hash = 0x811C9DC5; // FNV-1a
    for (uint32_t i = 0; i < file.size; i++) {
        hash = (hash ^ content[i]) * 0x01000193;
    }
    int elf = content[0] == 0x7F && content[1] == 'E' && content[2] == 'L' && content[3] == 'F';
    kfree(content);
    if (!elf || hash != 0xd5bc54c7) {
        kprintf("VAFS validation failed: HELLO.PAK /usr/bin/hello-world content differs (fnv1a 0x%x)\n", hash);
        return -1;
    }
    return 0;
}

// VAFSRAW.PAK and VAFSZST.PAK come from tests/vafs/make_test_pack.py, which writes 8KB blocks so
// reads cross block boundaries. Keep the content checks in sync with it.
static int vafs_check_generated_pack(struct vafs* vafs, const char* label) {
    struct vafs_node node;
    if (vafs_lookup(vafs, "/big.bin", &node) != 0 || node.type != VAFS_NODE_FILE || node.size != 300000) {
        kprintf("VAFS validation failed: %s /big.bin missing or the wrong size\n", label);
        return -1;
    }
    // odd sized pieces, so reads start and end at every kind of spot in the 37 blocks
    uint32_t piece = 7777;
    uint8_t* buffer = kmalloc(piece);
    if (buffer == NULL) {
        return -1;
    }
    uint32_t state = 0x12345678;
    for (uint32_t offset = 0; offset < node.size; offset += piece) {
        uint32_t expected = node.size - offset < piece ? node.size - offset : piece;
        if (vafs_read(vafs, &node, offset, buffer, piece) != (int)expected) {
            kprintf("VAFS validation failed: %s /big.bin short read at %u\n", label, offset);
            return -1;
        }
        for (uint32_t i = 0; i < expected; i++) {
            state = state * 1103515245u + 12345u;
            if (buffer[i] != (uint8_t)(state >> 16)) {
                kprintf("VAFS validation failed: %s /big.bin differs at byte %u\n", label, offset + i);
                return -1;
            }
        }
    }
    if (vafs_read(vafs, &node, node.size, buffer, piece) != 0) {
        kprintf("VAFS validation failed: %s read past the end of /big.bin returned data\n", label);
        return -1;
    }

    // the entry list of many/ is longer than one 8KB descriptor block
    struct vafs_dir it;
    struct vafs_node entry;
    uint32_t count = 0;
    int status;
    if (vafs_lookup(vafs, "/many", &node) != 0 || vafs_opendir(vafs, &node, &it) != 0) {
        kprintf("VAFS validation failed: %s cannot open /many\n", label);
        return -1;
    }
    while ((status = vafs_readdir(vafs, &it, &entry)) == 1) {
        count++;
    }
    if (status != 0 || count != 400) {
        kprintf("VAFS validation failed: %s /many lists %u entries, expected 400\n", label, count);
        return -1;
    }

    // files from the start, middle and end of that list: "file NNN\n", n % 5 + 1 times
    uint32_t picks[3] = {0, 199, 399};
    for (int p = 0; p < 3; p++) {
        uint32_t n = picks[p];
        char path[] = "/many/f000.txt";
        path[7] = '0' + n / 100;
        path[8] = '0' + (n / 10) % 10;
        path[9] = '0' + n % 10;
        uint32_t lines = n % 5 + 1;
        if (vafs_lookup(vafs, path, &node) != 0 || node.size != 9 * lines
            || vafs_read(vafs, &node, 0, buffer, node.size) != (int)node.size) {
            kprintf("VAFS validation failed: %s cannot read %s\n", label, path);
            return -1;
        }
        for (uint32_t line = 0; line < lines; line++) {
            const char* text = "file ";
            uint8_t* at = buffer + line * 9;
            for (int i = 0; i < 5; i++) {
                if (at[i] != (uint8_t)text[i]) {
                    status = -1;
                }
            }
            if (at[5] != path[7] || at[6] != path[8] || at[7] != path[9] || at[8] != '\n') {
                status = -1;
            }
        }
        if (status != 0) {
            kprintf("VAFS validation failed: %s %s content differs\n", label, path);
            return -1;
        }
    }

    if (vafs_lookup(vafs, "/deep/a/b/c/leaf.txt", &node) != 0 || vafs_read(vafs, &node, 0, buffer, 16) != 5
        || buffer[0] != 'l' || buffer[1] != 'e' || buffer[2] != 'a' || buffer[3] != 'f' || buffer[4] != '\n') {
        kprintf("VAFS validation failed: %s /deep/a/b/c/leaf.txt\n", label);
        return -1;
    }
    kfree(buffer);

    const char* target = "big.bin";
    if (vafs_lookup(vafs, "/link", &node) != 0 || node.type != VAFS_NODE_SYMLINK
        || vafs_read(vafs, &node, 0, &entry, 1) != -1) {
        kprintf("VAFS validation failed: %s /link is not a symlink\n", label);
        return -1;
    }
    for (int i = 0; i < 8; i++) {
        if (node.target[i] != target[i]) {
            kprintf("VAFS validation failed: %s /link points to %s\n", label, node.target);
            return -1;
        }
    }

    if (vafs_lookup(vafs, "/missing", &node) == 0 || vafs_lookup(vafs, "/big.bin/below-a-file", &node) == 0) {
        kprintf("VAFS validation failed: %s found a path that does not exist\n", label);
        return -1;
    }
    return 0;
}

int validate_vafs(struct bios_parameter_block* bpb) {
    const char* names[3] = {"HELLO   PAK", "VAFSZST PAK", "VAFSRAW PAK"};
    uint8_t* images[3];
    uint32_t sizes[3];
    for (int i = 0; i < 3; i++) {
        images[i] = vafs_load_file(bpb, names[i], &sizes[i]);
        if (images[i] == NULL) {
            kprintf("VAFS validation failed: could not load %s\n", names[i]);
            return -1;
        }
    }

    for (int i = 0; i < 3; i++) {
        struct vafs* vafs;
        if (vafs_mount(images[i], sizes[i], &vafs) != 0) {
            kprintf("VAFS validation failed: could not mount %s\n", names[i]);
            return -1;
        }
        int status = i == 0 ? vafs_check_bake_pack(vafs) : vafs_check_generated_pack(vafs, names[i]);
        vafs_unmount(vafs);
        if (status != 0) {
            return -1;
        }
    }

    // A damaged pack has to fail cleanly, not hang or hand back garbage
    uint32_t size = sizes[0];
    uint8_t* copy = kmalloc(size);
    if (copy == NULL) {
        return -1;
    }
    struct vafs* vafs;
    memcpy(copy, images[0], size);
    copy[0] ^= 0xFF;
    if (vafs_mount(copy, size, &vafs) == 0) {
        kprintf("VAFS validation failed: mounted a pack with a bad magic\n");
        return -1;
    }

    // a flipped byte in the compressed data block: the descriptors are fine, reading the file is not
    memcpy(copy, images[0], size);
    uint32_t data_offset = ((const struct vafs_header*)copy)->data_offset;
    copy[data_offset + sizeof(struct vafs_stream_header) + 100] ^= 0x01;
    struct vafs_node file;
    uint8_t byte;
    if (vafs_mount(copy, size, &vafs) != 0 || vafs_lookup(vafs, "/usr/bin/hello-world", &file) != 0) {
        kprintf("VAFS validation failed: a damaged data block broke the descriptors too\n");
        return -1;
    }
    if (vafs_read(vafs, &file, 0, &byte, 1) != -1) {
        kprintf("VAFS validation failed: read from a damaged data block succeeded\n");
        return -1;
    }
    vafs_unmount(vafs);

    // cut off in the middle of the data stream's header
    memcpy(copy, images[0], size);
    if (vafs_mount(copy, data_offset + 8, &vafs) == 0) {
        kprintf("VAFS validation failed: mounted a truncated pack\n");
        return -1;
    }

    kfree(copy);
    for (int i = 0; i < 3; i++) {
        kfree(images[i]);
    }
    kprintf("VAFS validation passed!\n");
    return 0;
}
