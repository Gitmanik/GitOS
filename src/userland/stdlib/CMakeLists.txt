file(GLOB_RECURSE C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE ASM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.asm")

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

add_library(stdlib STATIC ${ASM_OBJECTS} ${C_SOURCES} ${CPP_SOURCES})
target_include_directories(stdlib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(stdlib PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        LINKER_LANGUAGE CXX
)

target_link_options(stdlib PRIVATE
        -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
        -relocatable
)

set(STDLIB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
