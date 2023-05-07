cmake_minimum_required (VERSION 3.25)
set (CMAKE_CROSSCOMPILING 1)
set (CMAKE_C_COMPILER_WORKS 1)
set (CMAKE_CXX_COMPILER_WORKS 1)
set (CMAKE_VERBOSE_MAKEFILE ON)

project(GitOS)

set(KERNEL_BIN kernel.bin)
set(KERNEL_ELF kernel.elf)
set(STAGE1_BIN stage1.bin)
set(DISK_BIN disk.bin)
set(SRCROOT ${PROJECT_SOURCE_DIR})

add_subdirectory(${SRCROOT}/src/boot)
add_subdirectory(${SRCROOT}/src/kernel)

# kernel.bin target
add_custom_target(
    ${KERNEL_BIN}
    COMMAND ${CMAKE_OBJCOPY} -O binary ${KERNEL_ELF} ${KERNEL_BIN}
)
add_dependencies(${KERNEL_BIN} kernel)

# disk.bin target
add_custom_target(
    make_disk
    COMMAND bash ${SRCROOT}/scripts/make_disk.sh ${SRCROOT}/fs ${DISK_BIN} ${STAGE1_BIN} ${KERNEL_BIN}
    BYPRODUCTS ${DISK_BIN}
)
add_dependencies(make_disk ${KERNEL_BIN} stage1)