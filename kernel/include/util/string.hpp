#pragma once
#include <stddef.h>
#include <string.h>

struct string_buf {
    const char *data;
    size_t length;

    string_buf(const char *data) : data{data} {
        length = strlen(data);
    }
    string_buf(char *data) : string_buf{const_cast<const char *>(data)} {}

    string_buf(const char *data, size_t length) : data{data}, length{length} {}
    string_buf(char *data, size_t length) : string_buf{const_cast<const char *>(data), length} {}
};

#define INT_TO_STR_BUF_SIZE 21  // -2**63 with space for null byte

namespace str_util {
    // returns length of string
    size_t from(int val, char *out);
    size_t hex(void *val, char *out);
}
