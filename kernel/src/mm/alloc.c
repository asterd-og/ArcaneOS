#include <mm/alloc.h>
#include <mm/slab.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <kernel/kprintf.h>
#include <stdbool.h>

slab_cache_t *cache_bins[8];
bool alloc_initialized = false;

void alloc_init() {
	size_t obj_size = 8;
	for (int i = 0; i < 8; i++) {
		cache_bins[i] = slab_create_cache("alloc bin", obj_size + 4, NULL, NULL);
		obj_size *= 2;
	}
	alloc_initialized = true;
}

slab_cache_t *alloc_get_bin(size_t size) {
	if (size <= 8) return cache_bins[0];
	uint64_t cache = 64 - __builtin_clzll(size - 1) - 3;
	return cache_bins[cache];
}

void *alloc(size_t size) {
	if (size > 1024 || alloc_initialized == false) {
		uint64_t page_count = DIV_ROUND_UP(size, PAGE_SIZE);
		void *ptr = vmm_alloc(kernel_pagemap, page_count, MAP_READ | MAP_WRITE);

		*(uint32_t*)ptr = page_count * PAGE_SIZE;

		return (void*)((uint64_t)ptr + 4);
	}

	slab_cache_t *cache = alloc_get_bin(size);

	void *ptr = slab_alloc(cache);
	*(uint32_t*)ptr = cache->obj_size;

	return (void*)((uint64_t)ptr + 4);
}

void free(void *ptr) {
	void *off_ptr = (void*)((uint64_t)ptr - 4);

	uint32_t size = *(uint32_t*)off_ptr;

	if (size > 1024) {
		vmm_free(kernel_pagemap, off_ptr);

		return;
	}

	slab_free(ptr);
}
