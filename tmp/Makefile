
boot:
	make -C boot

cpu:
	make -C cpu

drivers:
	make -C drivers

kernel:
	make -C kernel

kernel_entry:
	make -C kernel_entry

libc:
	make -C libc

kernel.bin:
	i686-elf-ld -o kernel.bin -Ttext 0x1000 boot/kernel_entry.o kernel/kernel.o cpu/interrupt.o drivers/disk.o drivers/keyboard.o drivers/screen.o cpu/idt.o cpu/isr.o cpu/ports.o cpu/timer.o libc/mem.o libc/string.o --oformat binary

OS:
	cat boot/Stage1.bin kernel.bin > PuhaaOS-image.bin