#project setup
project(Kernel)


if(USE_DISKBUILDER)
    add_subdirectory(Kernel-Rust)
endif()


if(USE_DD)
    add_subdirectory(Kernel-Rust)
endif()


#add_library(kernel kernel.c)