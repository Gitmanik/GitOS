//
// Created by Pawel Reich on 1/15/25.
//

#pragma once

#include <stdint.h>

void execprocess(const char *path);
void get_process_arguments(int* argc, char*** argv);
void exit(int status);
void* malloc(int size);
void free(void* address);
void open_ipc(const char* filename, uint32_t packet_size, uint32_t count);