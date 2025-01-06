# Skip compiler checks
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
#set(CMAKE_VERBOSE_MAKEFILE ON)

cmake_minimum_required(VERSION 3.16)
project(GitOS C CXX ASM)

# Set the cross-compiler tools
set(TOOLS_DIR "/opt/cross/bin")
set(TARGET "i686-elf")
set(CMAKE_C_COMPILER "${TOOLS_DIR}/${TARGET}-gcc")
set(CMAKE_CXX_COMPILER "${TOOLS_DIR}/${TARGET}-g++")
set(CMAKE_OBJCOPY "${TOOLS_DIR}/${TARGET}-objcopy")
set(CMAKE_OBJDUMP "${TOOLS_DIR}/${TARGET}-objdump")
set(NASM "nasm")

# Compiler arguments
set(CMAKE_C_FLAGS "-g -std=gnu99 -ffreestanding -nostdlib -O0 -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS "-g -std=c++17 -ffreestanding -nostdlib -O0 -Wall -Wextra -Werror -fno-rtti -fno-exceptions -fno-sized-deallocation")

# Add subdirectories
add_subdirectory(src/boot)
add_subdirectory(src/kernel)
add_subdirectory(src/userland)

# Output for the final disk image
set(DISK_BIN "${CMAKE_CURRENT_BINARY_DIR}/disk.bin")

add_custom_target(disk ALL
        COMMAND rm -f ${DISK_BIN}
        COMMAND umount ${CMAKE_CURRENT_BINARY_DIR}/mnt || (exit 0)
        COMMAND rm -rf ${CMAKE_CURRENT_BINARY_DIR}/mnt

        COMMAND dd if=${STAGE1_BIN} >> ${DISK_BIN}
        COMMAND dd if=${KERNEL_BIN} >> ${DISK_BIN}
        COMMAND dd if=/dev/zero of=${DISK_BIN} seek=16777215 bs=1 count=1

        COMMAND mkdir ${CMAKE_CURRENT_BINARY_DIR}/mnt
        COMMAND mount -t vfat -o fat=16 -o uid=1000 -o gid=1000 ${DISK_BIN} ${CMAKE_CURRENT_BINARY_DIR}/mnt
        COMMAND cp -r ${CMAKE_SOURCE_DIR}/fs/. ${CMAKE_CURRENT_BINARY_DIR}/mnt/.
        COMMAND cp -r ${USERLAND_DESTINATION}/. ${CMAKE_CURRENT_BINARY_DIR}/mnt/.
        COMMAND umount ${CMAKE_CURRENT_BINARY_DIR}/mnt || (exit 0)
        COMMENT "Generating disk image"
)
add_dependencies(disk stage1_bin kernel_bin userland)