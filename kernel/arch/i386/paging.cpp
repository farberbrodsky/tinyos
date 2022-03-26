#include <kernel/util.hpp>
#include <kernel/paging.hpp>
#include <string.h>

// we have two kinds of memory:
// low memory (lmem), which is memory we need to know directly the physical address of (e.g. page tables)
// and high memory (hmem), which is mapped normally
// low memory goes up and high memory goes down

inline void *phys_of_lmem(void *addr) {
    return (void *)((uintptr_t)(addr) ^ 0xC0000000);
}

inline void *virt_of_lmem(void *addr) {
    return (void *)((uintptr_t)(addr) | 0xC0000000);
}

extern char _kernel_end;

constexpr uint32_t num_page_tables = 2;  // 1 is enough for kernel, 1 more as the first free page table
static uint32_t current_page_table = 0;

uint32_t initial_page_tables[num_page_tables][1024] __attribute__((aligned(4096)));
// yes this is how i allocate it, sue me
uint32_t page_dir[1024] __attribute__((aligned(4096)));  // ^^^ same here
uint32_t *free_page_table;                               // must always have one

// linked list heap implementation
// allocate both virtual and physical pages
static char *lmem_heap_start;
static char *lmem_heap_free;
// high memory
static char *hmem_heap_start;

constexpr void *align_page_up(void *addr) {
    return reinterpret_cast<void *>((((uint32_t)((char *)addr + 4095)) >> 12) << 12);
}

static inline void flush_tlb() {
    void *page_dir_phys = phys_of_lmem(page_dir);
    asm volatile("movl %0, %%cr3" :: "r" (page_dir_phys));
}

void paging::initialize(uintptr_t ram_amount) {
    // we already initialize the pages in boot.S
    // but I want to have full control of this in the kernel
    // this may be a way to support >4mb kernel later
    char *kernel_start = (char *)0xC0000000;  // start from here because bottom 1mb is useful too
    char *kernel_end = &_kernel_end;

    memset(page_dir, 0, 4096);

    for (char *page = kernel_start; page < kernel_end; page += PAGE_SIZE) {
        // TODO optimize it for this specific situation, this is like a generic page map
        uint32_t dir_index = (uintptr_t)page >> 22;
        uint32_t tab_index = (uintptr_t)page >> 12 & 0x03FF;

        uint32_t dir_entry = page_dir[dir_index];
        if (dir_entry & (uint)page_flag::present) {
            uint32_t *tab_addr = (uint32_t *)virt_of_lmem((void *)((dir_entry >> 12) << 12));
            tab_addr[tab_index] = (uint32_t)(phys_of_lmem(page)) | ((uint)page_flag::write | (uint)page_flag::present);
        } else {
            // need a new page table
            uint32_t *new_tab_addr = initial_page_tables[current_page_table++];
            memset(new_tab_addr, 0, 4096);
            new_tab_addr[tab_index] = (uint32_t)(phys_of_lmem(page)) | ((uint)page_flag::write | (uint)page_flag::present);
            page_dir[dir_index] = (uint32_t)(phys_of_lmem(new_tab_addr)) | ((uint)page_flag::write | (uint)page_flag::present);
        }
    }

    // set this as the page table
    flush_tlb();

    // initialize allocators
    lmem_heap_start = (char *)align_page_up(kernel_end);
    lmem_heap_free = nullptr;
    free_page_table = initial_page_tables[current_page_table++];
    hmem_heap_start = (char *)0xC0100000 + ram_amount;
}

void *paging::lookup_page(void *addr) {
    uint32_t dir_index = (uintptr_t)addr >> 22;
    uint32_t tab_index = (uintptr_t)addr >> 12 & 0x03FF;

    uint32_t dir_entry = page_dir[dir_index];
    if (dir_entry & (uint)page_flag::present) {
        uint32_t *table = (uint32_t *)virt_of_lmem((uint32_t *)((dir_entry >> 12) << 12));
        return (void *)(table[tab_index]);
    }
    return nullptr;
}

void *paging::lookup_physical_addr(void *addr) {
    return reinterpret_cast<void *>((reinterpret_cast<uintptr_t>(lookup_page(addr)) >> 12) << 12);
}

void paging::map_page(void *phys, void *virt, uint flags, bool is_page_table) {
    uint32_t dir_index = (uintptr_t)virt >> 22;
    uint32_t tab_index = (uintptr_t)virt >> 12 & 0x03FF;

    uint32_t dir_entry = page_dir[dir_index];
    uint32_t *tab_addr;
    if (dir_entry & (uint)page_flag::present) {
        tab_addr = (uint32_t *)virt_of_lmem((void *)((dir_entry >> 12) << 12));
        uint32_t tab_entry = tab_addr[tab_index];
        tab_addr[tab_index] = (uint32_t)phys | (uint32_t)flags;
        if ((uint32_t)tab_entry != tab_addr[tab_index]) {
            flush_tlb();  // we changed the mapping
        }
    } else {
        // allocate a new table
        if (!is_page_table) {
            tab_addr = (uint32_t *)(paging::allocate_lmem((uint)page_flag::write | (uint)page_flag::present));
            memset(tab_addr, 0, 4096);
            tab_addr[tab_index] = (uint32_t)phys | (uint32_t)flags;
            page_dir[dir_index] = (uint32_t)tab_addr | ((uint)page_flag::write | (uint)page_flag::present);
        } else {
            // this situation is when the page allocator is trying to make a page table for map_page
            // use free_page_table, and allocate a new one (which should be possible now)
            tab_addr = free_page_table;
            memset(tab_addr, 0, 4096);
            tab_addr[tab_index] = (uint32_t)phys | (uint32_t)flags;
            page_dir[dir_index] = (uint32_t)tab_addr | ((uint)page_flag::write | (uint)page_flag::present);
            // should be possible now, there are available addresses in a page table
            free_page_table = (uint32_t *)paging::allocate_lmem((uint)page_flag::write | (uint)page_flag::present);
        }
    }
    // don't need to flush tlb to add a new page
}

void paging::unmap_page(void *virt) {
    uint32_t dir_index = (uintptr_t)virt >> 22;
    uint32_t tab_index = (uintptr_t)virt >> 12 & 0x03FF;

    uint32_t dir_entry = page_dir[dir_index];
    kassert(dir_entry & (uint)page_flag::present);

    uint32_t *tab_addr = (uint32_t *)virt_of_lmem((void *)((dir_entry >> 12) << 12));

    kassert(tab_addr[tab_index] & (uint)page_flag::present);

    tab_addr[tab_index] = 0;
    flush_tlb();
}

void *paging::allocate_lmem(uint flags) {
    // step 1. get a virtual page address
    void *virt_page;
    if (lmem_heap_free != nullptr) {
        virt_page = lmem_heap_free;
        lmem_heap_free = (char *)(*((uintptr_t *)lmem_heap_free));
    } else {
        virt_page = lmem_heap_start;
        lmem_heap_start += PAGE_SIZE;
    }

    // step 2. map phys to virt
    paging::map_page(phys_of_lmem(virt_page), virt_page, flags, true);
    return virt_page;
}

void paging::free_lmem(void *virt) {
    // make it writable
    paging::map_page(phys_of_lmem(virt), virt, (uint)page_flag::write | (uint)page_flag::present, true);
    *((uintptr_t *)virt) = (uintptr_t)lmem_heap_free;
    lmem_heap_free = (char *)virt;
    // don't unmap - this is now part of the linked list
}
