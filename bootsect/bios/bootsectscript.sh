nasm -f bin -o mbr.bin mbr.asm
nasm -f bin -o Loader.bin Loader.asm
dd if=mbr.bin of=boot.img bs=512 count=1 conv=notrunc
dd if=Loader.bin of=boot.img bs=512 count=5 seek=1 conv=notrunc
qemu-system-x86_64 -cpu qemu64,pdpe1gb -drive format=raw,file=boot.img -D ./log.txt -monitor stdio -smp 1 -m 4096