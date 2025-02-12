//
// Created by root on 2/12/25.
//

#pragma once
#include <stdint-gcc.h>

class BMPFile {
    public:
        BMPFile(uint8_t* data, uint32_t sz);
        bool has_color_table() const;
        const uint32_t* get_raster() const;

        uint32_t get_pixel(uint32_t x, uint32_t y) const;
        uint32_t get_width() const;
        uint32_t get_height() const;

    private:
    struct BITMAPFILEHEADER {
        char signature[2];
        uint32_t file_size;
        uint32_t reserved;
        uint32_t offset;
    } __attribute__((__packed__));
    struct BITMAPINFOHEADER {
        uint32_t size;
        uint32_t width;
        uint32_t height;
        uint16_t planes;
        uint16_t bit_count;
        uint32_t compression;
        uint32_t image_size;
        uint32_t x_pixels_per_meter;
        uint32_t y_pixels_per_meter;
        uint32_t colors_used;
        uint32_t colors_important;
    } __attribute__((__packed__));


    const BITMAPFILEHEADER* m_file_header;
    const BITMAPINFOHEADER* m_info_header;
    const uint32_t* m_color_table;
    const uint32_t* m_raster_data;
};
