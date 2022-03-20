#pragma once
#include <stdint.h>

void __kassert_fail_internal(const char *assertion, const char *file, unsigned int line, const char *function);

#define kassert(expr) __kassert_internal(expr, #expr, __FILE__, __LINE__, __PRETTY_FUNCTION__);
inline void __kassert_internal(bool expr, const char *assertion, const char *file, unsigned int line, const char *function) {
    if (!expr) {
        __kassert_fail_internal(assertion, file, line, function);
    }
}
