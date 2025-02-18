//
// Created by Pawel Reich on 2/16/25.
//

#include "GitBoot.hpp"

#include <kernel.h>
#include <drivers/graphics/vbe/vbe_graphics.hpp>

void GitBoot::init(uint32_t magic, [[maybe_unused]] void* info_ptr) {
    
    if (magic != 1337)
        return;

    kprintf("Found GitBoot\n");
    Bootloader::m_bootloader = static_cast<Bootloader*>(this);

    memory_map_entry heap_entry{};

    int idx = 0;
    kprintf("Usable memory map:\n");
    while (bios_memory_map[idx].length_in_bytes > 0)
    {
        if (bios_memory_map[idx].type != 1)
            goto skip;

        kprintf("0x%p -> 0x%p, Size: %ldKB\n",
                (long)bios_memory_map[idx].base_address,
                (long)bios_memory_map[idx].base_address + (long)bios_memory_map[idx].length_in_bytes,
                (long)bios_memory_map[idx].length_in_bytes / 1024);

        if (bios_memory_map[idx].length_in_bytes > heap_entry.length_in_bytes)
            heap_entry = bios_memory_map[idx];

    skip:
        idx++;
    }

    m_base_address = heap_entry.base_address;
    m_len = heap_entry.length_in_bytes;

    const VbeModeInfo* modeInfo =reinterpret_cast<VbeModeInfo *>(0x2000);

    auto* framebuffer = reinterpret_cast<uint8_t*>(modeInfo->framebuffer);
    if (framebuffer == nullptr)
        return;
    ((VBEGraphics*) VBEGraphics::the())->setup(framebuffer, modeInfo->bpp, modeInfo->xRes, modeInfo->yRes);
    VBEGraphics::the()->clear_screen();
}

const char* GitBoot::get_cmdline() const
{
    static const char* cmdline = "";
    return cmdline;
}

const char* GitBoot::get_bootloader_name() const
{
    static const char* bootloader_name = "GitBoot";
    return bootloader_name;
}

uint64_t GitBoot::get_heap_base_address() const {
    return m_base_address;
}

uint64_t GitBoot::get_heap_size() const {
    return m_len;
}
