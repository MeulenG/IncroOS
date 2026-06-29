project(Kernel-C)

enable_language(C)

message(CHECK_START "Finding C Kernel In Project")
list(APPEND CMAKE_MESAGE_INDENT "  ")
unset(check_Kernel_C)

message(CHECK_START "Finding System Kernel Directory")
# do checks, but assume we find it
message(CHECK_PASS "Found Directory")

message(CHECK_START "Finding C Kernel In Directory")
# Check
message(CHECK_PASS "Found Kernel C File in Directory")

list(POP_BACK CMAKE_MESSAGE_INDENT)
if(check_Kernel_C)
    message(CHECK_FAIL "Missing File: ${check_Kernel_C}")
else()
    message(CHECK_PASS "Found C Kernel")
endif()

add_subdirectory(cpu)
add_subdirectory(drivers)
add_subdirectory(output)
add_subdirectory(memory)
add_subdirectory(lib)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/kernel.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -Wall -Wextra -c ${CMAKE_CURRENT_SOURCE_DIR}/kernel.c -o ${CMAKE_BINARY_DIR}/kernel.o
    COMMENT "Compiling C Kernel"
<<<<<<< Updated upstream
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kernel.c ${CMAKE_BINARY_DIR}/terminal.o ${CMAKE_BINARY_DIR}/boot_service.o ${CMAKE_BINARY_DIR}/serial.o ${CMAKE_BINARY_DIR}/pmm.o ${CMAKE_BINARY_DIR}/vmm.o ${CMAKE_BINARY_DIR}/kmalloc.o ${CMAKE_BINARY_DIR}/print.o ${CMAKE_BINARY_DIR}/string.o ${CMAKE_BINARY_DIR}/gdt.o ${CMAKE_BINARY_DIR}/idt.o ${CMAKE_BINARY_DIR}/interrupt_handler.o ${CMAKE_BINARY_DIR}/pic.o
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kernel.c ${CMAKE_BINARY_DIR}/terminal.o ${CMAKE_BINARY_DIR}/boot_service.o ${CMAKE_BINARY_DIR}/serial.o ${CMAKE_BINARY_DIR}/keyboard.o ${CMAKE_BINARY_DIR}/pmm.o ${CMAKE_BINARY_DIR}/vmm.o ${CMAKE_BINARY_DIR}/kmalloc.o ${CMAKE_BINARY_DIR}/print.o ${CMAKE_BINARY_DIR}/string.o ${CMAKE_BINARY_DIR}/gdt.o ${CMAKE_BINARY_DIR}/idt.o ${CMAKE_BINARY_DIR}/interrupt_handler.o ${CMAKE_BINARY_DIR}/pic.o
)

add_custom_target(Kernel ALL DEPENDS ${CMAKE_BINARY_DIR}/kernel.o ${CMAKE_BINARY_DIR}/terminal.o ${CMAKE_BINARY_DIR}/boot_service.o ${CMAKE_BINARY_DIR}/serial.o ${CMAKE_BINARY_DIR}/pmm.o ${CMAKE_BINARY_DIR}/vmm.o ${CMAKE_BINARY_DIR}/kmalloc.o ${CMAKE_BINARY_DIR}/print.o ${CMAKE_BINARY_DIR}/string.o ${CMAKE_BINARY_DIR}/gdt.o ${CMAKE_BINARY_DIR}/idt.o ${CMAKE_BINARY_DIR}/interrupt_handler.o ${CMAKE_BINARY_DIR}/pic.o)
add_dependencies(Kernel terminal BootService SerialDriver PMM VMM KMALLOC PrintLib StringLib GDT IDT InterruptHandler PIC)
=======
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kernel.c ${CMAKE_BINARY_DIR}/terminal.o ${CMAKE_BINARY_DIR}/boot_service.o ${CMAKE_BINARY_DIR}/serial.o ${CMAKE_BINARY_DIR}/keyboard.o ${CMAKE_BINARY_DIR}/vesa.o ${CMAKE_BINARY_DIR}/pmm.o ${CMAKE_BINARY_DIR}/vmm.o ${CMAKE_BINARY_DIR}/kmalloc.o ${CMAKE_BINARY_DIR}/print.o ${CMAKE_BINARY_DIR}/string.o ${CMAKE_BINARY_DIR}/gdt.o ${CMAKE_BINARY_DIR}/idt.o ${CMAKE_BINARY_DIR}/interrupt_handler.o ${CMAKE_BINARY_DIR}/pic.o
)

add_custom_target(Kernel ALL DEPENDS ${CMAKE_BINARY_DIR}/kernel.o ${CMAKE_BINARY_DIR}/terminal.o ${CMAKE_BINARY_DIR}/boot_service.o ${CMAKE_BINARY_DIR}/serial.o ${CMAKE_BINARY_DIR}/keyboard.o ${CMAKE_BINARY_DIR}/vesa.o ${CMAKE_BINARY_DIR}/pmm.o ${CMAKE_BINARY_DIR}/vmm.o ${CMAKE_BINARY_DIR}/kmalloc.o ${CMAKE_BINARY_DIR}/print.o ${CMAKE_BINARY_DIR}/string.o ${CMAKE_BINARY_DIR}/gdt.o ${CMAKE_BINARY_DIR}/idt.o ${CMAKE_BINARY_DIR}/interrupt_handler.o ${CMAKE_BINARY_DIR}/pic.o)
add_dependencies(Kernel terminal BootService SerialDriver KeyboardDriver VESADriver PMM VMM KMALLOC PrintLib StringLib GDT IDT InterruptHandler PIC)
>>>>>>> Stashed changes
add_custom_target(Kernel ALL DEPENDS ${CMAKE_BINARY_DIR}/kernel.o ${CMAKE_BINARY_DIR}/terminal.o ${CMAKE_BINARY_DIR}/boot_service.o ${CMAKE_BINARY_DIR}/serial.o ${CMAKE_BINARY_DIR}/keyboard.o ${CMAKE_BINARY_DIR}/pmm.o ${CMAKE_BINARY_DIR}/vmm.o ${CMAKE_BINARY_DIR}/kmalloc.o ${CMAKE_BINARY_DIR}/print.o ${CMAKE_BINARY_DIR}/string.o ${CMAKE_BINARY_DIR}/gdt.o ${CMAKE_BINARY_DIR}/idt.o ${CMAKE_BINARY_DIR}/interrupt_handler.o ${CMAKE_BINARY_DIR}/pic.o)
add_dependencies(Kernel terminal BootService SerialDriver KeyboardDriver PMM VMM KMALLOC PrintLib StringLib GDT IDT InterruptHandler PIC)
