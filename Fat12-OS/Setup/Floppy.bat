wsl dd if=/dev/zero of=myfloppy.img bs=512 count=2880
wsl mkfs.msdos -F 12 myfloppy.img
imdisk -a -f myfloppy.img -s 1440K -m A: -o fd
wsl dd if=../BrokenThorn/SysBoot/Stage1/Boot1.bin of=myfloppy.img bs=512 count=1 conv=notrunc
copy Fat12-OS\BrokenThorn\SysBoot\Stage2\KRNLDR.SYS A:\

pause