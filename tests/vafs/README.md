# Test packs for the VaFS driver

This folder holds what the kernel's VaFS test (`validate_vafs` in `kernel/drivers/vafs.c`) reads at boot. Three packs end up on the test disk:

- `HELLO.PAK` is `hello-world.pack` from this folder, a real pack built by bake. It's here so the driver gets tested against the real thing, not just against our own idea of the format.
- `VAFSZST.PAK` and `VAFSRAW.PAK` are generated during the build by `make_test_pack.py`, once compressed with zstd and once without compression.

## Looking inside a pack

`vafs_ls.py` prints everything in a pack, with a size and a hash for every file:

```
$ python3 vafs_ls.py hello-world.pack
# zstd, data blocks of 1048576 bytes
d /usr/
d /usr/bin/
f /usr/bin/hello-world 15960 fnv1a=0xd5bc54c7
```

That's where the numbers in the kernel test come from. Reading bake's packs needs the `zstd` command line tool.

## Rebuilding hello-world.pack

You shouldn't need to, but if you do: it was built with `bake build` from bake's own example recipe, `External/bake/examples/recipes/hello.yaml` (bake `fbed9d7`). bake needs its `cvd` daemon running for that, and `External/bake/tests/system/cases/hello-build.sh` shows the steps. If the new pack looks different, update the size and hash in `vafs_check_bake_pack` with what `vafs_ls.py` prints.

## Why we generate packs ourselves

bake's packs are tiny, so they never make the driver read across a block boundary, and that's where the tricky bugs live. libvafs comes with `mkvafs`, but it always compresses with aplib or brieflz, and our kernel only speaks zstd. So `make_test_pack.py` writes packs itself, with small 8KB blocks, a 300KB file, a directory of 400 files and a symlink. The kernel test rebuilds the same content and compares.

Since that's our own writer, it's worth checking it against the real thing now and then. libvafs' `unmkvafs` can unpack the uncompressed one:

```
python3 make_test_pack.py raw raw.pak
../../External/bake/build/bin/unmkvafs --out out raw.pak
```

All 402 files should come out identical. One quirk: `unmkvafs` mixes up two arguments when it recreates the symlink, so you'll find a stray `big.bin` link in the folder you ran it from instead of `out/link`. Delete it, it isn't ours.
