# Environmental Variables
include 				/home/puhaa/Desktop/PuhaaOS/buildbootpath

#NASM - Assembly Compiler

ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f



all			: 			$(BUILD_DIR_BOOT)/Stage1.bin

$(BUILD_DIR_BOOT)/Stage1.bin: Stage1.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@


clean:
	rm -rf $(BUILD_DIR_BOOT)/*.bin