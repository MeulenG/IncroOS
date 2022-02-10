# Environmental Variables
include 				/home/puhaa/Desktop/OMOS/buildbootpath

#NASM - Assembly Compiler

ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f



all			: 			$(BUILD_DIR_BOOT)/fat-stage1.bin

$(BUILD_DIR_BOOT)/fat-stage1.bin: fat-stage1.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@


clean:
	rm -rf $(BUILD_DIR_BOOT)/*.bin