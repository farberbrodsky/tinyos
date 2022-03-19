#pragma once
#include <stdint.h>
#include <util/string.hpp>

namespace tty {
    enum class color {
        black = 0,
        blue = 1,
        green = 2,
        cyan = 3,
        red = 4,
        magenta = 5,
        brown = 6,
        light_gray = 7,
        dark_gray = 8,
        light_blue = 9,
        light_green = 10,
        light_cyan = 11,
        light_red = 12,
        light_magenta = 13,
        light_brown = 14,
        white = 15
    };

    void initialize();
    void set_color(color fg, color bg);
    void putc(char c);
    void write(const string_buf &s);
}
