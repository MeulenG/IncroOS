# OS
My first OS written in C

nasm -f bin -o boot.bin boot.asm

This creates boot.bin which is your boot sector. The next step is to create a floppy disk image and place boot.bin in the first sector. You can do that with this:

dd if=/dev/zero of=floppy.img bs=1024 count=1440
dd if=boot.bin of=floppy.img seek=0 count=1 conv=notrunc


The first command simply makes a zero filled disk image equal to the size of a 1.44MB floppy (1024*1440 bytes). The second command places boot.bin into the first sector of floppy.img without truncating the rest of the file. seek=0 says seek to first sector (512 bytes is default size of a block for DD). count=1 specifies we only want to copy 1 sector (512 bytes) from boot.bin. conv=notrunc says that after writing to the output file, that the remaining disk image is to remain intact (not truncated).

After building a disk image as shown above, you can create an ISO image with these commands:

mkdir iso
cp floppy.img iso/
genisoimage -quiet -V 'MYOS' -input-charset iso8859-1 -o myos.iso -b floppy.img \
    -hide floppy.img iso/


The ISO image myos.iso that is generated can be booted. An example of using QEMU to launch such an image:

qemu-system-i386 -cdrom ./myos.iso
