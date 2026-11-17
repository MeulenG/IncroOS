#!/usr/bin/env python3
"""Lists what is inside a VaFS pack: every directory, file (with size and FNV-1a hash) and symlink.

The expected values in validate_vafs (kernel/drivers/vafs.c) come from this, and it parses the
format the same way the kernel driver does. Packs built by bake are zstd compressed, reading
those needs the zstd command line tool.

Usage: python3 vafs_ls.py <pack>
"""
import struct
import subprocess
import sys

VAFS_MAGIC = 0x3144524D
VAFS_VERSION = 0x00010000
STREAM_MAGIC = 0x314D5356
FILTER_GUID = bytes([0x91, 0x5D, 0xC2, 0x99, 0x99, 0xFA, 0x71, 0x4A,
                     0x9C, 0xB5, 0x96, 0x1A, 0xA9, 0x3D, 0xDF, 0xBB])
TYPE_FILE, TYPE_DIRECTORY, TYPE_SYMLINK = 1, 2, 3


def fnv1a(data):
    value = 0x811C9DC5
    for byte in data:
        value = ((value ^ byte) * 0x01000193) & 0xFFFFFFFF
    return value


class Stream:
    def __init__(self, pack, start, zstd):
        magic, self.block_size, table, count = struct.unpack_from("<4I", pack, start)
        if magic != STREAM_MAGIC:
            sys.exit(f"bad stream magic at {start}")
        self.pack, self.start, self.zstd = pack, start, zstd
        # LengthOnDisk, Offset (from the stream start), Crc, Flags, Reserved
        self.blocks = [struct.unpack_from("<IIIHH", pack, start + table + i * 16) for i in range(count)]
        self.cache = {}

    def block(self, index):
        if index not in self.cache:
            length, offset = self.blocks[index][0], self.blocks[index][1]
            raw = self.pack[self.start + offset:self.start + offset + length]
            if self.zstd:
                raw = subprocess.run(["zstd", "-d", "-c", "-q"], input=raw, capture_output=True,
                                     check=True).stdout
            self.cache[index] = raw
        return self.cache[index]

    def read(self, position, length):
        index, offset = position
        out = bytearray()
        while length > 0:
            block = self.block(index)
            if offset >= len(block):
                index, offset = index + 1, 0
                continue
            chunk = block[offset:offset + length]
            out += chunk
            offset += len(chunk)
            length -= len(chunk)
        return bytes(out), (index, offset)


def walk(descriptors, data, position, path):
    raw, position = descriptors.read(position, 4)
    count = struct.unpack("<I", raw)[0]
    for _ in range(count):
        raw, _ = descriptors.read(position, 4)
        kind, length = struct.unpack("<HH", raw)
        raw, position = descriptors.read(position, length)
        if kind == TYPE_FILE:
            index, offset, size, _ = struct.unpack_from("<IIII", raw, 4)
            name = raw[20:].decode()
            content, _ = data.read((index, offset), size)
            print(f"f {path}{name} {size} fnv1a=0x{fnv1a(content):08x}")
        elif kind == TYPE_DIRECTORY:
            index, offset, _ = struct.unpack_from("<III", raw, 4)
            name = raw[16:].decode()
            print(f"d {path}{name}/")
            walk(descriptors, data, (index, offset), f"{path}{name}/")
        elif kind == TYPE_SYMLINK:
            name_length, target_length = struct.unpack_from("<HH", raw, 4)
            name = raw[8:8 + name_length].decode()
            target = raw[8 + name_length:8 + name_length + target_length].decode()
            print(f"l {path}{name} -> {target}")
        else:
            sys.exit(f"unknown descriptor type {kind}")


def main():
    pack = open(sys.argv[1], "rb").read()
    magic, version, _, feature_count, _, _, desc_offset, data_offset, root_index, root_offset = \
        struct.unpack_from("<IIIHHIIIII", pack, 0)
    if magic != VAFS_MAGIC or version != VAFS_VERSION:
        sys.exit("not a VaFS pack")

    zstd = False
    position = 36
    for _ in range(feature_count):
        guid, length = pack[position:position + 16], struct.unpack_from("<I", pack, position + 16)[0]
        zstd = zstd or guid == FILTER_GUID
        position += length

    descriptors = Stream(pack, desc_offset, zstd)
    data = Stream(pack, data_offset, zstd)
    print(f"# {'zstd' if zstd else 'uncompressed'}, data blocks of {data.block_size} bytes")
    walk(descriptors, data, (root_index, root_offset), "/")


if __name__ == "__main__":
    main()
