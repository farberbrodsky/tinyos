#pragma once
#include <kernel/util.hpp>
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

    enum class mode {
        null = 0,
        hex = 1
    };

    struct hex {
        uintptr_t val;
        inline explicit hex(uintptr_t val) : val{val} {}
    };

    void initialize();
    void set_color(color fg, color bg);
    void write(char c);
    void write(int val);
    void write(hex val);

    inline void write(const string_buf &s) {
        for (size_t i = 0; i < s.length; i++) {
            write(s.data[i]);
        }
    }

    // variadic version
    inline void __write_rec() {}
    template <class T, class... Args>
    inline void __write_rec(T t, Args... args) {
        tty::write(t);
        tty::__write_rec(args...);
    }
    template <class... Args>
    inline void write(Args... args) {
        tty::__write_rec(args...);
    }
}
