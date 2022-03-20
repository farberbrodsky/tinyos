#pragma once
#include <kernel/util.hpp>

namespace paging {
    void initialize();
    void *get_physical_addr(void *addr);
    void map_page(void *phys, void *virt, unsigned int flags);

    enum class page_flag {
        present = 1,
        write = 1 << 1,
        user = 1 << 2,
        write_through = 1 << 3,
        cache_disable = 1 << 4,
        accessed = 1 << 5
    };
}
