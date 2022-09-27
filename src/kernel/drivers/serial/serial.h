#pragma once
#include <stdint.h>

#define COM1 0x3f8

int ser_Init(uint16_t port, uint16_t divisor);
void ser_PrintChar(uint16_t port, char c);
void ser_PrintString(uint16_t port, const char* str);
int ser_Received(uint16_t port);
char ser_ReadChar(uint16_t port);

