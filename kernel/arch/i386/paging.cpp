#include <kernel/util.hpp>
#include <kernel/paging.hpp>
#include <string.h>

// we have two kinds of memory:
// low memory (lmem), which is memory we need to know directly the physical address of (e.g. page tables)
// and high memory (hmem), which is mapped normally
// low memory goes up and high memory goes down

inline uintptr_t phys_of_lmem(void *addr) {
    return (uintptr_t)(addr) ^ 0xC0000000;
}

inline void *virt_of_lmem(void *addr) {
    return (void *)((uintptr_t)(addr) | 0xC0000000);
}

inline void *virt_of_lmem(uintptr_t addr) {
    return (void *)(addr | 0xC0000000);
}

extern char _kernel_end;

constexpr uint32_t num_page_tables = 3;  // 1 is enough for kernel, 2 more as the free page tables
static uint32_t current_page_table = 0;

uint32_t initial_page_tables[num_page_tables][1024] __attribute__((aligned(4096)));
// yes this is how i allocate it, sue me
uint32_t page_dir[1024] __attribute__((aligned(4096)));  // ^^^ same here
uint32_t *free_page_table[2];                            // must always have two - one to use now, and one to be able to allocate a replacement
                                                         // we never use 1 except for when we replace 0 with it

// linked list heap implementation
// allocate both virtual and physical pages
static char *lmem_heap_start;
static char *lmem_heap_free;
// high memory
static uintptr_t hmem_phys_heap_start;
static uintptr_t hmem_phys_heap_free;
static char *hmem_heap_start;
static char *hmem_heap_free;

constexpr void *align_page_up(void *addr) {
    return (void *)((((uintptr_t)((char *)addr + 4095)) >> 12) << 12);
}

constexpr void *align_4mb_up(void *addr) {
    return (void *)((((uintptr_t)((char *)addr + 4095)) >> 22) << 22);
}

constexpr void *align_page_down(void *addr) {
    return (void *)((((uintptr_t)addr) >> 12) << 12);
}

static inline void flush_tlb() {
    uintptr_t page_dir_phys = phys_of_lmem(page_dir);
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
            tab_addr[tab_index] = phys_of_lmem(page) | ((uint)page_flag::write | (uint)page_flag::present);
        } else {
            // need a new page table
            uint32_t *new_tab_addr = initial_page_tables[current_page_table++];
            memset(new_tab_addr, 0, 4096);
            new_tab_addr[tab_index] = phys_of_lmem(page) | ((uint)page_flag::write | (uint)page_flag::present);
            page_dir[dir_index] = phys_of_lmem(new_tab_addr) | ((uint)page_flag::write | (uint)page_flag::present);
        }
    }

    // set this as the page table
    flush_tlb();

    // initialize allocators
    lmem_heap_start = (char *)align_page_up(kernel_end);
    lmem_heap_free = nullptr;
    free_page_table[0] = initial_page_tables[current_page_table++];
    free_page_table[1] = initial_page_tables[current_page_table++];
    kassert(current_page_table <= num_page_tables);

    hmem_phys_heap_start = (uintptr_t)align_page_down((void *)(0x100000 + ram_amount));  // start at the very top
    hmem_phys_heap_free = (uintptr_t)(-PAGE_SIZE);
    hmem_heap_start = (char *)align_4mb_up((void *)(lmem_heap_start + ram_amount));  // goes down
    hmem_heap_free = nullptr;
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

void paging::map_page(uintptr_t phys, void *virt, uint flags) {
    uint32_t dir_index = (uintptr_t)virt >> 22;
    uint32_t tab_index = (uintptr_t)virt >> 12 & 0x03FF;

    uint32_t dir_entry = page_dir[dir_index];
    uint32_t *tab_addr;
    if (dir_entry & (uint)page_flag::present) {
        tab_addr = (uint32_t *)virt_of_lmem((void *)((dir_entry >> 12) << 12));
        uint32_t tab_entry = tab_addr[tab_index];
        if (phys == 0xFFFFFFFF) {
            phys = (tab_entry >> 12) << 12;
        }
        tab_addr[tab_index] = (uint32_t)phys | (uint32_t)flags;
        if ((uint32_t)tab_entry != tab_addr[tab_index]) {
            flush_tlb();  // we changed the mapping
        }
    } else {
        // need a new table - use free_page_table[0], and we'll make a new free_page_table[1] to replace it
        if (free_page_table[0] != nullptr) {
            tab_addr = free_page_table[0];
            free_page_table[0] = free_page_table[1];
            free_page_table[1] = nullptr;
        } else {
            kassert(free_page_table[1] != nullptr);
            tab_addr = free_page_table[1];
            free_page_table[1] = nullptr;
        }

        memset(tab_addr, 0, 4096);
        tab_addr[tab_index] = phys | (uint32_t)flags;
        page_dir[dir_index] = phys_of_lmem(tab_addr) | ((uint)page_flag::write | (uint)page_flag::present);

        // getting new free page tables should be possible now, due to the behavior of lmem allocator:
        // needing a new free_page_table only happens (up to) once every 1024 allocations
        if (free_page_table[1] == nullptr)
            free_page_table[1] = (uint32_t *)paging::allocate_lmem((uint)page_flag::write | (uint)page_flag::present);
        if (free_page_table[0] == nullptr)
            free_page_table[0] = (uint32_t *)paging::allocate_lmem((uint)page_flag::write | (uint)page_flag::present);
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
    // TODO check for whole empty page tables and free them
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
        kassert(phys_of_lmem(virt_page) < hmem_phys_heap_start);
    }

    // step 2. map phys to virt
    paging::map_page(phys_of_lmem(virt_page), virt_page, flags);
    return virt_page;
}

void paging::free_lmem(void *virt) {
    // make it writable
    paging::map_page(phys_of_lmem(virt), virt, (uint)page_flag::write | (uint)page_flag::present);
    *((uintptr_t *)virt) = (uintptr_t)lmem_heap_free;
    lmem_heap_free = (char *)virt;
    // don't unmap - this is now part of the linked list
}

void *paging::allocate_virt() {
    void *virt_page;
    if (hmem_heap_free != nullptr) {
        virt_page = hmem_heap_free;
        hmem_heap_free = (char *)(*((uintptr_t *)hmem_heap_free));
    } else {
        hmem_heap_start -= PAGE_SIZE;
        virt_page = hmem_heap_start;
        kassert(virt_page > lmem_heap_start);
    }
    return virt_page;
}

void *paging::allocate(uint flags) {
    void *virt_page = paging::allocate_virt();
    // allocate physical in the same way
    uintptr_t phys_page;
    if (hmem_heap_free != nullptr) {
        phys_page = hmem_phys_heap_free;
        hmem_phys_heap_free = (*((uintptr_t *)hmem_phys_heap_free));
    } else {
        hmem_phys_heap_start -= PAGE_SIZE;
        phys_page = hmem_phys_heap_start;
    }

    paging::map_page(phys_page, virt_page, flags);
    return virt_page;
}

void paging::free(void *virt) {
    // make it writable
    paging::map_page(0xFFFFFFFF, virt, (uint)page_flag::write | (uint)page_flag::present);
    *((uintptr_t *)virt) = (uintptr_t)lmem_heap_free;
    lmem_heap_free = (char *)virt;
    // don't unmap - this is now part of the linked list
}
