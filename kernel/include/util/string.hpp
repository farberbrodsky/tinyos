#pragma once
#include <stddef.h>
#include <string.h>

struct string_buf {
    const char *data;
    size_t length;

    string_buf(const char *data) : data{data} {
        length = strlen(data);
    }

    string_buf(const char *data, size_t length) : data{data}, length{length} {}
};
