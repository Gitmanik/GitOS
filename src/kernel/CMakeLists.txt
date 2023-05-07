set (SRCROOT ${PROJECT_SOURCE_DIR}/src/kernel)

set (KERNEL_ELF kernel)

set (CMAKE_C_FLAGS "-ffreestanding -O0 -nostdlib -Wall -Wextra -I${SRCROOT} -Wno-maybe-uninitialized")
set (CMAKE_C_STANDARD 11)
set (CMAKE_EXE_LINKER_FLAGS "-T ${SRCROOT}/../linker.ld ${CMAKE_C_FLAGS}" CACHE STRING "" FORCE)

# Enable NASM assembly integration
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)
enable_language(ASM_NASM)

file (GLOB_RECURSE C_SOURCES ${SRCROOT}/*.c)
file (GLOB_RECURSE C_HEADERS ${SRCROOT}/*.h)
file (GLOB_RECURSE ASM_KERNEL_SOURCES ${SRCROOT}/*.asm)
list (REMOVE_ITEM ASM_KERNEL_SOURCES ${SRCROOT}/pm_entry.asm)
list (INSERT ASM_KERNEL_SOURCES 0 ${SRCROOT}/pm_entry.asm)

add_executable(${KERNEL_ELF})

# Build kernel.elf
target_sources(${KERNEL_ELF} PRIVATE ${ASM_KERNEL_SOURCES} ${C_SOURCES})
target_include_directories(${KERNEL_ELF} PRIVATE ${SRCROOT})