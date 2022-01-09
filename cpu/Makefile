include 				/home/puhaa/Desktop/PuhaaOS/buildpath.txt


# Environmental Variables
CC  		= 			/home/puhaa/opt/cross/bin/i686-elf-gcc
GDB 		= 			/home/puhaa/opt/cross/bin/i686-elf-gdb



# -g: Use debugging symbols in gcc
CFLAGS 	 	= 			-g
GCC_ARGS 	= 			-ffreestanding
GCC_ARGS 	+= 			-fda
GCC_ARGS 	+= 			-Wall
GCC_ARGS 	+= 			-Wextra
GCC_ARGS 	+= 			-fno-exceptions
GCC_ARGS 	+= 			-m32


all			: 			$(BUILD_DIR)/interrupt.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/isr.o $(BUILD_DIR)/port.o $(BUILD_DIR)/timer.o



$(BUILD_DIR)/interrupt.o: interrupt.asm
	${ASMC} interrupt.asm ${ASMC_ARGS} elf -o $@

$(BUILD_DIR)/idt.o: idt.c idt.h
	${CC} ${CFLAGS} ${GCC_ARGS} -c idt.c -o $@

$(BUILD_DIR)/isr.o: isr.c isr.h
	${CC} ${CFLAGS} ${GCC_ARGS} -c isr.c -o $@

$(BUILD_DIR)/port.o: port.c port.h
	${CC} ${CFLAGS} ${GCC_ARGS} -c port.c -o $@

$(BUILD_DIR)/timer.o: timer.c timer.h
	${CC} ${CFLAGS} ${GCC_ARGS} -c timer.c -o $@

clean:
	rm $(BUILD_DIR)/*