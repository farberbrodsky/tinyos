#include <kernel/util.hpp>
#include <kernel/tty.hpp>

extern "C" {
    void kernel_main(void);
}

void kernel_main(void) {
    tty::initialize();
    tty::write("Hello world!\nAAAAAAAA");

    int x = 3;
    kassert(x == 1);
    while (1) asm("hlt");
}
