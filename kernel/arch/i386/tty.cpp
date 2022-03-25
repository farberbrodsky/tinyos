#include <stdint.h>
#include <kernel/tty.hpp>

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static volatile uint16_t *VGA_MEMORY = (uint16_t *)0xc00b8000;

static size_t t_x;
static size_t t_y;
static uint8_t t_color;
static volatile uint16_t *t_buf;
static uint16_t internal_buf[VGA_WIDTH * VGA_HEIGHT];
size_t internal_buf_i;  // ring buffer

static tty::mode t_mode;

static inline uint8_t vga_color(tty::color fg, tty::color bg) {
    return (uint8_t)fg | ((uint8_t)bg << 4);
}

static inline uint16_t vga_entry(unsigned char code, uint8_t color) {
    return (uint16_t)code | (uint16_t)color << 8;
}

static inline void set_char(unsigned char code, uint8_t color, size_t x, size_t y) {
    t_buf[y * VGA_WIDTH + x] = vga_entry(code, color);
    internal_buf[((y + internal_buf_i) % VGA_HEIGHT) * VGA_WIDTH + x] = vga_entry(code, color);
}

void tty::initialize() {
    t_x = 0;
    t_y = 0;
    internal_buf_i = 0;
    t_mode = mode::null;
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

void tty::add_mode(tty::mode m) {
    t_mode = (mode)((uint)t_mode | (uint)m);
}

void tty::remove_mode(tty::mode m) {
    t_mode = (mode)((uint)t_mode & ~(uint)m);
}

static void tty_scroll() {
    if (++internal_buf_i == VGA_HEIGHT) internal_buf_i = 0;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            t_buf[y * VGA_WIDTH + x] = internal_buf[((y + internal_buf_i) % VGA_HEIGHT) * VGA_WIDTH + x];
        }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        set_char(' ', t_color, x, VGA_HEIGHT - 1);
    }
}

void tty::write(char c) {
    if (c == '\n') {
        t_x = 0;
        if (t_y == VGA_HEIGHT - 1) {
            tty_scroll();
        } else {
            t_y++;
        }
    } else {
        unsigned char uc = c;
        set_char(uc, t_color, t_x++, t_y);
        if (t_x == VGA_WIDTH) {
            t_x = 0;
            if (t_y == VGA_HEIGHT - 1) {
                tty_scroll();
            } else {
                t_y++;
            }
        }
    }
}

void tty::write(int val) {
    char buf[INT_TO_STR_BUF_SIZE];
    if ((uint)t_mode & (uint)mode::hex) {
        str_util::hex((void *)val, buf);
    } else {
        str_util::from(val, buf);
    }
    tty::write(buf);
}
