BUILD_DIR=../build

BOOT_SRCS := $(wildcard *.asm)
BOOT_OBJS := $(patsubst %.asm, $(BUILD_DIR)/%.o, $(BOOT_SRCS))

all: $(BOOT_OBJS)



$(BUILD_DIR)/%.bin: %.asm
	nasm -f bin $< -o $@


clean:
	rm $(BUILD_DIR)/*