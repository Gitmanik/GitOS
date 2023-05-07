set(SRCROOT ${PROJECT_SOURCE_DIR}/src/boot)

set(CMAKE_ASM_NASM_OBJECT_FORMAT bin)
enable_language(ASM_NASM)

set(STAGE1_BIN stage1.bin)

add_custom_target(
    stage1
    WORKING_DIRECTORY ${SRCROOT}
    COMMAND ${CMAKE_ASM_NASM_COMPILER} -f bin ${SRCROOT}/boot.asm -o ${CMAKE_BINARY_DIR}/${STAGE1_BIN}
    BYPRODUCTS ${STAGE1_BIN}
)
