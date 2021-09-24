nasm -f bin -o boot.bin Boot1.asm


dd if=/dev/zero of=floppy.img bs=1024 count=1440
dd if=boot.bin of=floppy.img seek=0 count=1 conv=notrunc

mkdir iso
cp floppy.img iso/
genisoimage -quiet -V 'MYOS' -input-charset iso8859-1 -o myos.iso -b floppy.img \
    -hide floppy.img iso/

qemu-system-i386 -cdrom ./myos.iso