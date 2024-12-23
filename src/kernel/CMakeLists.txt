# Output for the kernel
set(KERNEL_ELF "${CMAKE_CURRENT_BINARY_DIR}/kernel.elf")
set(KERNEL_ELF ${KERNEL_ELF} PARENT_SCOPE)
set(KERNEL_BIN "${CMAKE_CURRENT_BINARY_DIR}/kernel.bin")
set(KERNEL_BIN ${KERNEL_BIN} PARENT_SCOPE)

# Source files
file(GLOB_RECURSE C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE ASM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.asm")
list(REMOVE_ITEM ASM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/kernel.asm)
list(INSERT ASM_SOURCES 0 ${CMAKE_CURRENT_SOURCE_DIR}/kernel.asm)

# Assemble assembly files
set(ASM_OBJECTS)
foreach(ASM_FILE ${ASM_SOURCES})
    get_filename_component(ASM_FILE_NAME ${ASM_FILE} NAME_WE)
    set(ASM_OBJECT ${CMAKE_CURRENT_BINARY_DIR}/${ASM_FILE_NAME}.asm.o)
    add_custom_command(
            OUTPUT ${ASM_OBJECT}
            COMMAND nasm -g -f elf ${ASM_FILE} -o ${ASM_OBJECT}
            DEPENDS ${ASM_FILE}
            COMMENT "Assembling ${ASM_FILE}"
    )
    list(APPEND ASM_OBJECTS ${ASM_OBJECT})
endforeach()

# Define the kernel target
add_executable(kernel ${ASM_OBJECTS} ${C_SOURCES} ${CPP_SOURCES})
target_include_directories(kernel PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(kernel PROPERTIES
        OUTPUT_NAME "kernel.elf"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Linker script
target_link_options(kernel PRIVATE
        -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
)

# Generate the kernel binary
add_custom_command(
        OUTPUT ${KERNEL_BIN}
        COMMAND ${CMAKE_OBJCOPY} -O binary ${KERNEL_ELF} ${KERNEL_BIN}
        COMMAND nm ${KERNEL_ELF} | sort | sed 's/\\ A\\ / /g' > ${CMAKE_CURRENT_BINARY_DIR}/bochs_symbols.sym
        DEPENDS kernel
        COMMENT "Generating kernel binary"
)

add_custom_target(kernel_bin DEPENDS ${KERNEL_BIN})