#pragma once
#include <kernel/util.hpp>
#define PAGE_SIZE 4096

namespace paging {
    void initialize(uintptr_t ram_amount);
    // includes flags and such
    void *lookup_page(void *addr);
    // only addr
    void *lookup_physical_addr(void *addr);
    void map_page(void *phys, void *virt, uint flags, bool is_page_table = false);
    void *allocate_lmem(uint flags);
    void *allocate(uint flags);

    enum class page_flag : uint {
        present = 1,
        write = 1 << 1,
        user = 1 << 2,
        write_through = 1 << 3,
        cache_disable = 1 << 4,
        accessed = 1 << 5,
        global = 1 << 8
    };
}
