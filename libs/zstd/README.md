# zstd decoder

Single-file zstd decompressor for the kernel, used to read bake `.pack` data blocks.

## Upstream files (unmodified)

| File | Source |
|---|---|
| `zstddeclib.c` | generated from zstd v1.5.7 by `build/single_file_libs/combine.py` |
| `zstd.h`, `zstd_errors.h` | `lib/` of zstd v1.5.7 |
| `LICENSE` | zstd's BSD license (zstd is dual licensed BSD or GPLv2, this repo uses BSD) |

Version 1.5.7 matches what bake builds against (`External/bake/cmake/external/zstd/zstd.cmake`).

Source archive: `https://github.com/facebook/zstd/archive/refs/tags/v1.5.7.tar.gz`,
sha256 `37d7284556b20954e56e1ca85b80226768902e2edabd3b649e9e72c0c9012ee3`.

`zstddeclib.c` sha256 `5eb38abe0a7eea13674b312a006df72002d4192f0f0b4d09e6f03fb927a1f3d0`. To regenerate it, from the extracted archive:

```sh
cd build/single_file_libs
python3 combine.py -r ../../lib -x legacy/zstd_legacy.h -o zstddeclib.c zstddeclib-in.c
```

## Ours

- `zstd_kernel.c`: includes `zstddeclib.c` with `ZSTD_malloc`/`ZSTD_calloc`/`ZSTD_free` mapped to the kernel heap, so zstd never includes `stdlib.h`. Also holds `validate_zstd`, the boot self-test.
- `zstd_test_vector.h`: the frame the self-test decodes, written by `make_test_vector.py`.
- `CMakeLists.txt`: builds `zstd_kernel.o` without SSE/MMX (the kernel never enables them) and with `-fno-reorder-functions` (see the comment there).

The decoder needs `memcpy`, `memmove` and `memset`, which come from `libs/libkernel/string.c`.
