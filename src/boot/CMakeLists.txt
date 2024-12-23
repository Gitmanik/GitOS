set(STAGE1_BIN "${CMAKE_CURRENT_BINARY_DIR}/stage1.bin")
set(STAGE1_BIN ${STAGE1_BIN} PARENT_SCOPE)

add_custom_command(
        OUTPUT ${STAGE1_BIN}
        COMMAND nasm -I ${CMAKE_CURRENT_SOURCE_DIR} -f bin ${CMAKE_CURRENT_SOURCE_DIR}/boot.asm -o ${STAGE1_BIN}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/boot.asm
        COMMENT "Assembling bootloader"
)

add_custom_target(stage1_bin DEPENDS ${STAGE1_BIN})