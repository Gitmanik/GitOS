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
set(CMAKE_C_FLAGS "-g -std=gnu99 -ffreestanding -fno-omit-frame-pointer -nostdlib -O0 -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS "-g -std=c++17 -ffreestanding -nostdlib -O0 -Wall -Wextra -Werror -fno-rtti -fno-exceptions -fno-omit-frame-pointer -fno-sized-deallocation -fno-threadsafe-statics -fno-use-cxa-atexit")

# Add subdirectories
add_subdirectory(src/boot)
add_subdirectory(src/kernel)
add_subdirectory(src/userland)

# Output for the final disk image
set(DISK_BIN "${CMAKE_CURRENT_BINARY_DIR}/disk.bin")
set(MOUNT_DIR "${CMAKE_CURRENT_BINARY_DIR}/mnt")
set(BOOT_DIR "${MOUNT_DIR}/boot")
set(FS_DIR "${CMAKE_SOURCE_DIR}/fs")


add_custom_target(gitboot ALL
        COMMAND sh ${CMAKE_SOURCE_DIR}/scripts/install_gitboot.sh ${DISK_BIN} ${MOUNT_DIR} ${FS_DIR} ${USERLAND_DESTINATION} ${KERNEL_BIN} ${STAGE1_BIN}
)
add_dependencies(gitboot stage1_bin kernel_bin userland)
