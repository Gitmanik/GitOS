#pragma once
#include "drivers/text_mode/text_mode.h" // To be removed - hack until OS gets proper journal

void kernel_main();
void kernel_halt();
void kernel_message(char* message, enum TEXT_MODE_COLORS col);
void kernel_panic(char* message);