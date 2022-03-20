#include <kernel/util.hpp>
#include <kernel/tty.hpp>

void __kassert_fail_internal(const char *assertion, const char *file, unsigned int line, const char *function) {
    char line_buf[INT_TO_STR_BUF_SIZE];
    str_util::from(line, line_buf);
    tty::write("\nASSERTION FAILED: ", assertion, " in file ", file, ':', line_buf, " in function ", function);
    while (1) asm("nop");  // TODO something better than this
}
