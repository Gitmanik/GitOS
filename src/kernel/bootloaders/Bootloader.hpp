//
// Created by Pawel Reich on 2/18/25.
//

#pragma once

#include <stdint-gcc.h>

#pragma once

class Bootloader {
    friend class GitBoot;

    public:
      virtual void init(uint32_t magic, void* info_ptr) = 0;
      virtual ~Bootloader() = default;
      static Bootloader* the() {
          return m_bootloader;
      };

      virtual const char* get_cmdline() const = 0;
      virtual const char* get_bootloader_name() const = 0;

      virtual uint64_t get_heap_base_address() const = 0;
      virtual uint64_t get_heap_size() const = 0;

    private:
        static Bootloader* m_bootloader;
};