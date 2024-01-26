cmake_minimum_required(VERSION 3.22.1)

project("OS")

enable_language(ASM_NASM)

option(MFOS "Building Target MFOS" ON)
option(Barebones "Building Barebones" OFF)

set (CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
# Let's check for dependencies
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

if(MFOS)
    set(boot-dir ${CMAKE_SOURCE_DIR}/boot)
    set(kernel-dir ${CMAKE_SOURCE_DIR}/kernel)

    add_subdirectory(boot)
    add_subdirectory(kernel)

    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/kernel.bin
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND x86_64-linux-gnu-ld -T ${CMAKE_SOURCE_DIR}/linker.ld -o ${CMAKE_BINARY_DIR}/kernel.bin ${CMAKE_BINARY_DIR}/kernel.o ${CMAKE_BINARY_DIR}/entry.o
        DEPENDS ${CMAKE_BINARY_DIR}/kernel.o ${CMAKE_BINARY_DIR}/entry.o
    )
    add_custom_target(MyOS ALL DEPENDS ${CMAKE_BINARY_DIR}/kernel.bin)

    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/kernel.raw
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND x86_64-linux-gnu-objcopy -O binary ${CMAKE_BINARY_DIR}/kernel.bin ${CMAKE_BINARY_DIR}/kernel.raw
        DEPENDS ${CMAKE_BINARY_DIR}/kernel.bin
    )
    add_custom_target(KernelBin ALL DEPENDS ${CMAKE_BINARY_DIR}/kernel.raw)

    add_custom_command(
        OUTPUT disk.img
        COMMAND cd .. && external/diskbuilder/build/osbuilder os.yaml --target img
        DEPENDS ${CMAKE_BINARY_DIR}/BOOT.SYS ${CMAKE_BINARY_DIR}/STAGE2.SYS ${CMAKE_BINARY_DIR}/kernel.bin ${CMAKE_BINARY_DIR}/kernel.raw
    )
    add_custom_target(disk-image ALL DEPENDS disk.img)

    # Explicit dependencies
    add_dependencies(MyOS Kernel KernelEntry Boot stage2)
    add_dependencies(KernelBin MyOS)
    add_dependencies(disk-image MyOS KernelBin)
endif(MFOS)


if(Barebones)
    set(boot-dir
        ${CMAKE_SOURCE_DIR}/SysBoot
    )
    set(kernel-dir
        ${CMAKE_SOURCE_DIR}/Kernel
    )

    message(CHECK_START "Attempting to find system bootloaders")
    if(EXISTS "${boot-dir}" AND IS_DIRECTORY "${boot-dir}")
        message("Found System BootLoaders")
    else()
        message(CHECK_FAIL "${boot-dir} doesn't exist")
    endif()
    add_subdirectory(SysBoot)

    message(CHECK_START "Attempting to find kernel")
    if(EXISTS "${kernel-dir}" AND IS_DIRECTORY "${kernel-dir}")
        message("Found Kernel")
    else()
        message(CHECK_FAIL "${kernel-dir} doesn't exist")
    endif()

    add_custom_command(OUTPUT boot.o
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND i686-elf-as boot.s -o boot.o
        COMMENT "Compiling bootstrap boot.s"
        DEPENDS boot.s
    )
    add_custom_target(Boot ALL DEPENDS boot.o)

    add_subdirectory(Kernel)

    add_custom_command(OUTPUT myos.bin
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND i686-elf-gcc -T ${CMAKE_SOURCE_DIR}/linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o Kernel/Kernel-C/kernel.o -lgcc
        DEPENDS ${CMAKE_SOURCE_DIR}/linker.ld
    )
    add_custom_target(MyOS ALL DEPENDS myos.bin)

    add_custom_target(CreateDirectories ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/isodir/boot/grub
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/myos.bin isodir/boot/myos.bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/grub/grub.cfg isodir/boot/grub/grub.cfg
        DEPENDS ${CMAKE_SOURCE_DIR}/myos.bin
    )

    add_custom_target(CreateISO ALL
        COMMAND grub-mkrescue -o myos.iso isodir
        DEPENDS CreateDirectories
    )

    add_dependencies(CreateISO CreateDirectories)

    add_custom_target(Clean
        COMMAND ${CMAKE_COMMAND} -E remove -f myos.bin
        COMMAND ${CMAKE_COMMAND} -E remove -f myos.iso
        COMMAND ${CMAKE_COMMAND} -E remove_directory -f isodir
    )

    add_custom_target(FullClean
        COMMAND ${CMAKE_COMMAND} -E remove -f myos.bin
        COMMAND ${CMAKE_COMMAND} -E remove -f myos.iso
        COMMAND ${CMAKE_COMMAND} -E remove_directory -f isodir
    )

    # Add the dependencies to ensure proper order of execution
    add_dependencies(Clean CreateISO)
    add_dependencies(FullClean CreateISO)

    # Specify that "make Clean" and "make FullClean" do not produce any output
    set_target_properties(Clean PROPERTIES OUTPUT_NAME "")
    set_target_properties(FullClean PROPERTIES OUTPUT_NAME "")
endif(Barebones)