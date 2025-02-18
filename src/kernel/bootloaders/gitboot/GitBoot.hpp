//
// Created by Pawel Reich on 2/16/25.
//

#pragma once
#include <stdint-gcc.h>
#include "../Bootloader.hpp"

class GitBoot : public Bootloader {

  public:
    void init(uint32_t magic, void* info_ptr) override;
    const char* get_cmdline() const override;
    const char* get_bootloader_name() const override;
    uint64_t get_heap_base_address() const override;
    uint64_t get_heap_size() const override;

    uint64_t m_base_address;
    uint64_t m_len;

  private:
    struct memory_map_entry
    {
      uint64_t base_address;
      uint64_t length_in_bytes;
      uint32_t type;
    } __attribute__ ((packed));

    memory_map_entry* bios_memory_map = reinterpret_cast<memory_map_entry*>(0x500);

    struct VbeModeInfo {
      uint16_t attributes;
      uint8_t winA, winB;
      uint16_t granularity, winSize, segmentA, segmentB;
      uint32_t realFctPtr;
      uint16_t pitch;  // Bytes per scanline
      uint16_t xRes, yRes;
      uint8_t xCharSize, yCharSize, planes, bpp, banks;
      uint8_t memoryModel, bankSize, imagePages;
      uint8_t reserved0;
      uint8_t redMask, redPosition;
      uint8_t greenMask, greenPosition;
      uint8_t blueMask, bluePosition;
      uint8_t reservedMask, reservedPosition;
      uint8_t directColorAttributes;
      uint32_t framebuffer;  // Framebuffer address
      uint32_t offScreenMemOffset;
      uint16_t offScreenMemSize;
      uint8_t reserved1[206];
    } __attribute__((packed));

    bool m_gitboot = false;
};

