#include <kernel/util.hpp>
#include <kernel/tty.hpp>
#include <kernel/paging.hpp>
#include <kernel/multiboot.h>

extern char _kernel_start;
extern "C" {
    void kernel_main(multiboot_info_t *mbd, unsigned int magic);
}

void kernel_main(multiboot_info_t *mbd, unsigned int magic) {
    mbd = (multiboot_info_t *)((char *)mbd + 0xC0000000);
    kassert(magic == MULTIBOOT_BOOTLOADER_MAGIC);
    kassert(mbd->flags >> 6 & 0x1);  // check that there is a memory map from grub bootloader

    uintptr_t ram_amount = 0;
    for (uint i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
        multiboot_memory_map_t *mmmt = (multiboot_memory_map_t *)(mbd->mmap_addr + 0xC0000000 + i);
        if (mmmt->addr == 0x100000) {
            ram_amount = mmmt->len;
        }
    }

    kassert(ram_amount != 0);

    tty::initialize();
    paging::initialize(ram_amount);

    tty::write("Hello world!\nAAAAAAAA\n");
    tty::add_mode(tty::mode::hex);
    tty::write((int)ram_amount, " bytes of ram\n");
    tty::remove_mode(tty::mode::hex);

    uint32_t *page = (uint32_t *)paging::allocate_lmem((uint)paging::page_flag::write | (uint)paging::page_flag::present);
    page[5] = 7;

    kassert(13 == 37);
    while (1) asm("pause");
}
