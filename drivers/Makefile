# Environmental Variables
CC  		= 			/home/puhaa/opt/cross/bin/i686-elf-gcc
GDB 		= 			/home/puhaa/opt/cross/bin/i686-elf-gdb
include 				/home/puhaa/Desktop/PuhaaOS/buildpath


# -g: Use debugging symbols in gcc
CFLAGS 	 	= 			-g
GCC_ARGS 	= 			-ffreestanding
GCC_ARGS 	+= 			-Wall
GCC_ARGS 	+= 			-Wextra
GCC_ARGS 	+= 			-Werror
GCC_ARGS 	+= 			-fno-exceptions
GCC_ARGS 	+= 			-m32
GCC_ARGS 	+= 			-fno-builtin
GCC_ARGS 	+= 			-nostdlib
GCC_ARGS 	+= 			-nostdinc
GCC_ARGS 	+= 			-fno-stack-protector
GCC_ARGS 	+= 			-nostartfiles
GCC_ARGS 	+= 			-nodefaultlibs


all			: 			$(BUILD_DIR)/keyboard.o $(BUILD_DIR)/screen.o $(BUILD_DIR)/disk.o

$(BUILD_DIR)/keyboard.o: keyboard.c keyboard.h
	${CC} ${CFLAGS} ${GCC_ARGS} -c keyboard.c -o $@

$(BUILD_DIR)/screen.o: screen.c screen.h
	${CC} ${CFLAGS} ${GCC_ARGS} -c screen.c -o $@

$(BUILD_DIR)/disk.o: disk.c disk.h
	${CC} ${CFLAGS} ${GCC_ARGS} -c disk.c -o $@

clean:
	rm $(BUILD_DIR)/*