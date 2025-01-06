add_custom_target(userland)

set(USERLAND_BINARIES)
set(USERLAND_DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/dest)
set(USERLAND_DESTINATION ${USERLAND_DESTINATION} PARENT_SCOPE)

function(GITOS_PROGRAM name)
    MESSAGE("Registering GitOS program: ${name}")
    file(GLOB_RECURSE C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
    file(GLOB_RECURSE CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
    file(GLOB_RECURSE ASM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.asm")

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

    add_executable(${name} ${C_SOURCES} ${CPP_SOURCES} ${ASM_OBJECTS})

    target_link_options(${name} PRIVATE
            -T ${CMAKE_CURRENT_SOURCE_DIR}/../linker.ld
            -ffreestanding -O0 -nostdlib -fpic
    )

    target_include_directories(${name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    set_target_properties(${name} PROPERTIES
            OUTPUT_NAME "${name}.elf"
            RUNTIME_OUTPUT_DIRECTORY ${USERLAND_DESTINATION}
            LINKER_LANGUAGE CXX
    )

    list(APPEND USERLAND_PROGRAMS ${name})

    set(EXECUTABLE_PATH "${USERLAND_DESTINATION}/${name}.elf")
    list(APPEND USERLAND_BINARIES ${EXECUTABLE_PATH})
    set(USERLAND_BINARIES ${USERLAND_BINARIES} PARENT_SCOPE)

endfunction()

add_subdirectory(blank)
add_dependencies(userland blank)
