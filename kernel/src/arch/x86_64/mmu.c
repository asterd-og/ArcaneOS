#include <arch/mmu.h>
#include <mm/pmm.h>
#include <lib/string.h>
#include <kernel/kprintf.h>

#define GET_LEVEL(addr, level) ((addr >> (12 + (level * 9))) & 0x1ff)
#define EXISTS(x) ((uint64_t)x & MAP_READ)

uint64_t *mmu_new_level(uint64_t *level, uint64_t entry) {
	uint64_t *new_level = pmm_alloc();
	memset(HIGHER_HALF(new_level), 0, PAGE_SIZE);
	level[entry] = (uint64_t)new_level | MAP_READ | MAP_WRITE | MAP_USER;
	return new_level;
}

void mmu_map(pagemap_t *pagemap, uint64_t virt, uint64_t phys, uint64_t flags) {
	// Traverse all the page map levels and create a new one if it didnt find one.

	uint64_t *current_level = pagemap->top_level;
	for (uint64_t level = 3; level > 0; level--) {
		uint64_t level_idx = GET_LEVEL(virt, level);
		uint64_t *next_level = (uint64_t*)current_level[level_idx];
		if (!EXISTS(next_level))
			next_level = mmu_new_level(current_level, level_idx);
		current_level = HIGHER_HALF(GET_ADDR_FROM_PAGE(next_level));
	}

	uint64_t page_idx = GET_LEVEL(virt, 0);
	current_level[page_idx] = phys | flags;
}

uint64_t mmu_get_page(pagemap_t *pagemap, uint64_t virt) {
	uint64_t *current_level = pagemap->top_level;
	for (uint64_t level = 3; level > 0; level--) {
		uint64_t level_idx = GET_LEVEL(virt, level);
		uint64_t *next_level = (uint64_t*)current_level[level_idx];
		if (!EXISTS(next_level))
			return 0;
		current_level = HIGHER_HALF(GET_ADDR_FROM_PAGE(next_level));
	}

	uint64_t page_idx = GET_LEVEL(virt, 0);
	return current_level[page_idx];
}

void mmu_load_pagemap(pagemap_t *pagemap) {
	__asm__ volatile ("mov %0, %%cr3" : : "r"((uint64_t)LOWER_HALF(pagemap->top_level)) : "memory");
}
