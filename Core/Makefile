BUILD_DIR	=			../build/OS

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f


all			: 			$(BUILD_DIR)/kernel.bin kernel.bin

$(BUILD_DIR)/kernel.bin: kernel.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@


kernel.bin: kernel.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@

clean:
	rm $(BUILD_DIR)/*.bin
	rm -rf kernel.bin