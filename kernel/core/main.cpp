#include <kernel/tty.hpp>

extern "C" {
    void kernel_main(void);
}

void kernel_main(void) {
    tty::initialize();
    tty::write("Hello world!\nAAAAAAAA");
}
