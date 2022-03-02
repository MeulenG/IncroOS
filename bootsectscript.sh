nasm -f bin -o mbr.bin mbr.asm
nasm -f bin -o Loader.bin Loader.asm
nasm -f elf64 -o kernel.o kernel_entry.asm
nasm -f elf64 -o trapa.o trap.asm
nasm -f elf64 -o liba.o lib.asm
gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c main_kernel.c
gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c trap.c
gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c print.c
ld -nostdlib -T link.lds -o kernel kernel.o main_kernel.o trapa.o trap.o liba.o print.o
objcopy -O binary kernel kernel.bin
dd if=mbr.bin of=boot.img bs=512 count=1 conv=notrunc
dd if=Loader.bin of=boot.img bs=512 count=5 seek=1 conv=notrunc
dd if=kernel_entry.bin of=boot.img bs=512 count=100 seek=6 conv=notrunc
qemu-system-x86_64 -cpu qemu64,pdpe1gb -drive format=raw,file=boot.img -D ./log.txt -monitor stdio -smp 1 -m 4096