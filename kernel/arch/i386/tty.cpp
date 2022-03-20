#include <stdint.h>
#include <kernel/tty.hpp>

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static volatile uint16_t *VGA_MEMORY = (uint16_t *)0xc03ff000;

static size_t t_x;
static size_t t_y;
static uint8_t t_color;
static volatile uint16_t *t_buf;

static inline uint8_t vga_color(tty::color fg, tty::color bg) {
    return (uint8_t)fg | ((uint8_t)bg << 4);
}

static inline uint16_t vga_entry(unsigned char code, uint8_t color) {
    return (uint16_t)code | (uint16_t)color << 8;
}

static inline void set_char(unsigned char code, uint8_t color, size_t x, size_t y) {
    t_buf[y * VGA_WIDTH + x] = vga_entry(code, color);
}

void tty::initialize() {
    t_x = 0;
    t_y = 0;
    set_color(color::light_gray, color::black);
    t_buf = VGA_MEMORY;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
            set_char(' ', t_color, x, y);
		}
	}
}

void tty::set_color(color fg, color bg) {
    t_color = vga_color(fg, bg);
}

void tty::write(char c) {
    if (c == '\n') {
        t_x = 0;
        if (++t_y == VGA_HEIGHT) {
            // TODO implement scrolling
            t_y = 0;
        }
    } else {
        unsigned char uc = c;
        set_char(uc, t_color, t_x++, t_y);
        if (t_x == VGA_WIDTH) {
            t_x = 0;
            if (++t_y == VGA_HEIGHT) {
                // TODO implement scrolling
                t_y = 0;
            }
        }
    }
}

void tty::write(const string_buf &s) {
    for (size_t i = 0; i < s.length; i++) {
        write(s.data[i]);
    }
}
