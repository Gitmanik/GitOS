//
// Created by Pawel Reich on 1/26/25.
//
#pragma once
#include <stdint-gcc.h>
#include <fs/pipe/PipeFS.hpp>

class PS2Mouse {
    public:
        void handle_cycle();
        void wait(uint8_t type) const;
        void write(uint8_t byte) const;
        uint8_t read() const;

        static PS2Mouse* instance();

        struct mouse_packet { // TODO: Make this smaller
            int32_t x;
            int32_t y;
            unsigned char buttons;
        };
        uint32_t pipe_size = 128;

    private:
        PS2Mouse();
        int mouse_cycle = 0;
        char mouse_byte[3] = {0};
        PipeFS* pipe;

        const int MOUSE_IRQ = 12;
        const int MOUSE_PORT = 0x60;
        const int MOUSE_STATUS = 0x64;
        const int MOUSE_ABIT = 0x02; // Input buffer status
        const int MOUSE_BBIT = 0x01; // Output buffer status
        const int MOUSE_WRITE = 0xD4;
        const int MOUSE_F_BIT = 0x20; // Check mouse data validity
        const int MOUSE_V_BIT = 0x08; // Packet validity

        const int TIMEOUT = 100000;
};
