#pragma once
#include "drivers/text_mode/text_mode.h" // To be removed - hack until OS gets proper journal
#include "common/io.h"
#define BochsBreak() outw(0x8A00,0x8A00); outw(0x8A00,0x08AE0);

void kernel_main();
void kernel_halt();
void kernel_message(char* message, enum TEXT_MODE_COLORS col);
void kernel_debug(char* message);
void kernel_panic(char* message);