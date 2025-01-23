#pragma once

#include "drivers/graphics/graphics.hpp"

class TextModeGraphics : public Graphics {
public:
    // Returns the singleton instance implementing Graphics
    static Graphics* the();

    void print_char(char c) override;
    void print_char_color(char c, TEXT_MODE_COLOR color) override;

    void print_string(const char* str) override;
    void print_string_color(const char* str, TEXT_MODE_COLOR color) override;

    void clear_screen() override;

    void set_text_color(TEXT_MODE_COLOR color) override;
    void set_cursor(uint32_t x, uint32_t y) override;

    uint32_t get_cursor_x() override;
    uint32_t get_cursor_y() override;
    TEXT_MODE_COLOR get_current_color() override;

private:
    // Helper function to convert a character + color into a 16-bit entry
    uint16_t make_char(char c, TEXT_MODE_COLOR color);

    // Scroll the screen by 'amount' lines
    void scroll_screen(int amount);

public:
    ~TextModeGraphics() override {};

private:
    // We assume 80x25 text mode
    static constexpr int TEXT_MODE_WIDTH  = 80;
    static constexpr int TEXT_MODE_HEIGHT = 25;

    // Pointer to the VGA text-mode buffer
    uint16_t* video_mem = reinterpret_cast<uint16_t*>(0xB8000);

    // Current cursor position and current color
    uint32_t current_x = 0;
    uint32_t current_y = 0;
    TEXT_MODE_COLOR current_fg = WHITE;
};
