#include <kernel/util.hpp>
#include <util/string.hpp>

size_t str_util::from(int val, char *out) {
    if (val == 0) {
        out[0] = '0';
        out[1] = '\0';
        return 1;
    }

    size_t i = 0;
    size_t negative = 0;

    if (val < 0) {
        val = -val;
        negative = 1;
    }

    while (val != 0) {
        int remainder = val % 10;
        val = val / 10;
        out[i++] = '0' + remainder;
    }

    if (negative) {
        out[i++] = '-';
    }

    // reverse the string
    for (size_t j = negative; j < i; j++) {
        char tmp = out[j];
        out[j] = out[i - j - 1 + negative];
        out[i - j - 1 + negative] = tmp;
    }

    out[i] = '\0';
    return i;
}
