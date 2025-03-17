//
// Created by Pawel Reich on 2/15/25.
//

#pragma once
#include <stdint-gcc.h>
#include "../Bootloader.hpp"

#include "multiboot.h"


class Multiboot : public Bootloader {
public:
    void init(uint32_t magic, void* info_ptr) override;
    const char* get_cmdline() const override;
    const char* get_bootloader_name() const override;
    uint64_t get_heap_base_address() const override;
    uint64_t get_heap_size() const override;
    bool is_multiboot() const;

    struct multiboot2_header {
        uint32_t magic;
        uint32_t architecture;
        uint32_t header_length;
        uint32_t checksum;
        multiboot_header_tag_framebuffer framebuffer_tag;
    } __attribute__((packed));


    uint64_t m_base_address = 0;
    uint64_t m_len = 0;

private:
    __attribute__((section(".multiboot2"), used)) static const multiboot2_header m_mb2_header;

    char* m_cmdline;
    char* m_bootloader_name;
    bool m_is_multiboot;

    void parse_tags(multiboot_tag* tag);

};
