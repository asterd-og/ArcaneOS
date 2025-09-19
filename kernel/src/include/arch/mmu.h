#pragma once

#include <stdint.h>
#include <stddef.h>
#include <mm/vmm.h>

#if defined(__x86_64__)
#define PAGE_SIZE 4096
#define MAP_READ 1
#define MAP_WRITE 2
#define MAP_USER 4
#define MAP_NX (1ull << 63)
#define GET_ADDR_FROM_PAGE(x) (typeof(x))((uint64_t)x & 0x000ffffffffff000)
#elif defined(__aarch64__)
#define PAGE_SIZE 4096
#endif

void mmu_map(pagemap_t *pagemap, uint64_t virt, uint64_t phys, uint64_t flags);
uint64_t mmu_get_page(pagemap_t *pagemap, uint64_t virt);
void mmu_load_pagemap(pagemap_t *pagemap);
