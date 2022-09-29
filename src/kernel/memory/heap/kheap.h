#pragma once
#include <stddef.h>
#include <stdint.h>

int kheap_init(void* start_address, uint32_t size);
void* kmalloc(size_t size);
void* kzalloc(size_t size);
void kfree(void* ptr);