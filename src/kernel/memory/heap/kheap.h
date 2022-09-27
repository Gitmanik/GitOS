#pragma once
#include <stddef.h>

void kheap_init(void* start_address, uint32_t size);
void* kmalloc(size_t size);
void* kfree(void* ptr);