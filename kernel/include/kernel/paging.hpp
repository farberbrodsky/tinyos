#pragma once
#include <kernel/util.hpp>
#define PAGE_SIZE 4096

namespace paging {
    void initialize(uintptr_t ram_amount);
    // includes flags and such
    void *lookup_page(void *addr);
    // only addr
    void *lookup_physical_addr(void *addr);
    void map_page(uintptr_t phys, void *virt, uint flags);  // phys=0xFFFFFFFF for keeping same phys
    void unmap_page(void *virt);
    void *allocate_lmem(uint flags);  // allocates both physical and virtual addresses, in low memory (identity)
    void *allocate(uint flags);       // allocates both physical and virtual addresses
    void *allocate_virt();            // allocates only a virtual address, returns it for use in map_page
    void free_lmem(void *virt);
    void free(void *virt);

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
