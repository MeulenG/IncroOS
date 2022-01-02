BUILD_DIR=../build

BOOT_SRCS := $(wildcard *.asm)
BOOT_OBJS := $(patsubst %.asm, $(BUILD_DIR)/%.o, $(BOOT_SRCS))

all: $(BOOT_OBJS)

# Environmental Variables
CC = /home/puhaa/opt/cross/bin/i686-elf-gcc
GDB = /home/puhaa/opt/cross/bin/i686-elf-gdb

$(BUILD_DIR)/%.o: %.asm
	nasm $< -f bin -o $@

# Default Rule
os-image.bin: boot/bootsect.bin kernel.bin
	cat $^ > os-image.bin


kernel.bin: boot/kernel_entry.o ${OBJ}
	i686-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary
clean:
	rm $(BUILD_DIR)/*