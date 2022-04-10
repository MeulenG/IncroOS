BUILD_DIR	=			../build/bootloader

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f


all			: 			$(BUILD_DIR)/fat-stage1.bin	STAGE1.SYS

$(BUILD_DIR)/fat-stage1.bin: fat-stage1.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@


STAGE1.SYS: fat-stage1.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@

clean:
	rm $(BUILD_DIR)/*.bin
	rm -rf STAGE1.SYS