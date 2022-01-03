BUILD_DIR=../build

BOOT_SRCS := $(wildcard *.asm)
BOOT_OBJS := $(patsubst %.asm, $(BUILD_DIR)/%.o, $(BOOT_SRCS))

all: $(BOOT_OBJS)

# Environmental Variables
CC = /home/puhaa/opt/cross/bin/i686-elf-gcc
GDB = /home/puhaa/opt/cross/bin/i686-elf-gdb

$(BUILD_DIR)/%.o: %.asm
	nasm $< -f bin -o $@

clean:
	rm $(BUILD_DIR)/*