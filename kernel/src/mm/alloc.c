#include <mm/alloc.h>
#include <mm/slab.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <kernel/kprintf.h>

slab_cache_t *cache_bins[8];

void alloc_init() {
	size_t obj_size = 8;
	for (int i = 0; i < 8; i++) {
		cache_bins[i] = slab_create_cache("alloc bin", obj_size, NULL, NULL);
		obj_size *= 2;
	}
}

slab_cache_t *alloc_get_bin(size_t size) {
	if (size <= 8) return cache_bins[0];
	uint64_t cache = 64 - __builtin_clzll(size - 1) - 3;
	kprintf(LOG_INFO "Allocator: Selected bin %d (size %d).\n", cache, cache_bins[cache]->obj_size);
	return cache_bins[cache];
}

void *alloc(size_t size) {
	if (size >= 1024)
		return vmm_alloc(kernel_pagemap, DIV_ROUND_UP(size, PAGE_SIZE), MAP_READ | MAP_WRITE);
	slab_cache_t *cache = alloc_get_bin(size);
	return slab_alloc(cache);
}

void free(void *ptr) {
	slab_free(ptr);
}
