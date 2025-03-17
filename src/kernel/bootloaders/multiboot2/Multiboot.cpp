//
// Created by Pawel Reich on 2/15/25.
//

#include "Multiboot.hpp"

#include "multiboot.h"
#include <kernel.h>
#include <drivers/graphics/vbe/vbe_graphics.hpp>


const Multiboot::multiboot2_header Multiboot::m_mb2_header = {
    .magic = MULTIBOOT2_HEADER_MAGIC,
    .architecture = 0,
    .header_length = sizeof(multiboot2_header),
    .checksum = -(MULTIBOOT2_HEADER_MAGIC + 0 + sizeof(multiboot2_header)),
    .framebuffer_tag = {
        .type = MULTIBOOT_HEADER_TAG_FRAMEBUFFER,
        .flags = 0,
        .size = sizeof(multiboot_header_tag_framebuffer),
        .width = 1024,
        .height = 768,
        .depth = 32,
   }
};

void Multiboot::init(uint32_t magic, void* info_ptr)
{
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
    {
        m_is_multiboot = false;
        return;
    }
    m_is_multiboot = true;
    Bootloader::m_bootloader = static_cast<Bootloader*>(this);

    uint32_t info_size = *static_cast<uint32_t*>(info_ptr);
    uint32_t info_reserved = *(static_cast<uint32_t*>(info_ptr) + 1);

    multiboot_tag *tag = reinterpret_cast<multiboot_tag*>(reinterpret_cast<uintptr_t>(info_ptr) + 8);

    kdebug("Multiboot2 size: %d, reserved: %d", info_size, info_reserved);

    parse_tags(tag);
}

uint64_t Multiboot::get_heap_base_address() const {
    return m_base_address;
}

uint64_t Multiboot::get_heap_size() const {
    return m_len;
}

const char* Multiboot::get_cmdline() const {
    return m_cmdline;
}

const char* Multiboot::get_bootloader_name() const {
    return m_bootloader_name;
}

bool Multiboot::is_multiboot() const {
    return m_is_multiboot;
}

void Multiboot::parse_tags(multiboot_tag* tag)
{
    bool end_tag = false;
    while (!end_tag) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_CMDLINE: {
                m_cmdline = reinterpret_cast<multiboot_tag_string*>(tag)->string;
                break;
            }

            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
                m_bootloader_name = reinterpret_cast<multiboot_tag_string*>(tag)->string;
                break;
            }

            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
                auto* tag_fb = reinterpret_cast<multiboot_tag_framebuffer*>(tag);
                kdebug("Multiboot: Initializing framebuffer: %dx%dx%d", tag_fb->common.framebuffer_width, tag_fb->common.framebuffer_height, tag_fb->common.framebuffer_bpp);
                static_cast<VBEGraphics*>(VBEGraphics::the())->setup(reinterpret_cast<uint8_t*>(tag_fb->common.framebuffer_addr), tag_fb->common.framebuffer_bpp, tag_fb->common.framebuffer_width, tag_fb->common.framebuffer_height);
                VBEGraphics::the()->clear_screen();
                break;
            }

            case MULTIBOOT_TAG_TYPE_MMAP: {
                auto* tag_mmap = reinterpret_cast<multiboot_tag_mmap*>(tag);

                uint32_t entries = (tag_mmap->size - sizeof(multiboot_tag_mmap)) / tag_mmap->entry_size;
                auto *entry = (multiboot_mmap_entry *)((uintptr_t)tag_mmap + sizeof(multiboot_tag_mmap));

                for (uint32_t i = 0; i < entries; i++) {
                    if (entry->type == MULTIBOOT_MEMORY_AVAILABLE && entry->len > m_len) {
                        m_base_address = entry->addr;
                        m_len = entry->len;
                    }
                    entry = (multiboot_mmap_entry *)((uintptr_t)entry + tag_mmap->entry_size);
                }
                break;
            }

            case MULTIBOOT_TAG_TYPE_END: {
                end_tag = true;
                break;
            }

            default: {
                kdebug("Unknown Multiboot2 tag type: %d", tag->type);
                break;
            }
        }
        tag = (multiboot_tag *)((uintptr_t)tag + ((tag->size + 7) & ~7));
    }
}
