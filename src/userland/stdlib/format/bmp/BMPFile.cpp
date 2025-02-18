//
// Created by Paweł Reich on 2/12/25.
//

#include "BMPFile.hpp"

#include <stdint-gcc.h>
extern "C" {
#include <stdio.h>
}

BMPFile::BMPFile(uint8_t* data, uint32_t sz) {

    if (sz < (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))) {
        debug_printf("BMPFile::BMPFile(): invalid size\n");
        return;
    }

    m_file_header = (BITMAPFILEHEADER*) data;
    m_info_header = (BITMAPINFOHEADER*) (data + sizeof(BITMAPFILEHEADER));
    m_color_table = (uint32_t*) (m_info_header + m_info_header->size);
    m_raster_data = (uint32_t*) (data + m_file_header->offset);

    if (m_file_header->signature[0] != 'B' || m_file_header->signature[1] != 'M') {
        debug_printf("BMPFile::BMPFile(): invalid signature\n");
    }
    if (m_file_header->file_size != sz) {
        debug_printf("BMPFile::BMPFile(): invalid size: real %d, expected %d", sz, m_file_header->file_size);
    }

    if (m_info_header->compression != 0 && m_info_header->compression != 3) {
        debug_printf("BMPFile::BMPFile(): invalid compression: %d\n", m_info_header->compression);
    }

    switch (m_info_header->bit_count) {
        case 1:
        case 4:
        case 8:
        break;

        case 24:
        case 32:
            break;
        default:
            debug_printf("BMPFile::BMPFile(): invalid bit_count: %d", m_info_header->bit_count);
            break;
    }
}

bool BMPFile::has_color_table() const {
    return m_info_header->bit_count <= 8;
}

const uint32_t* BMPFile::get_raster() const {
    return m_raster_data;
}

uint32_t BMPFile::get_pixel(uint32_t x, uint32_t y) const {
    if (x >= m_info_header->width || y >= m_info_header->height) {
        debug_printf("BMPFile::BMPFile(): invalid pixel coordinate: %d %d\n", x, y);
        return 0;
    }

    uint32_t row = m_info_header->height - 1 - y;
    uint32_t width = m_info_header->width;

    if (m_info_header->bit_count == 24) {
        const uint8_t* raster = reinterpret_cast<const uint8_t*>(m_raster_data);
        uint32_t row_stride = ((width * 3 + 3) & ~3);
        const uint8_t* pixel_ptr = raster + row * row_stride + x * 3;

        uint8_t blue  = pixel_ptr[0];
        uint8_t green = pixel_ptr[1];
        uint8_t red   = pixel_ptr[2];
        return (0xFF << 24) | (red << 16) | (green << 8) | blue;
    }

    if (m_info_header->bit_count == 32) {
        return m_raster_data[row * width + x];
    }

    return 0;
}

uint32_t BMPFile::get_width() const {
    return m_info_header->width;
}

uint32_t BMPFile::get_height() const {
    return m_info_header->height;
}
