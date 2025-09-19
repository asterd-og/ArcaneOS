#include <mm/slab.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <arch/mmu.h>
#include <lib/string.h>
#include <kernel/kprintf.h>
#include <stddef.h>

slab_cache_t *slab_mem = NULL;
slab_cache_t *slab_cache_mem = NULL;

void slab_init() {
	// Creates slab_mem and slab_cache_mem.
	slab_mem = slab_create_cache("slab_mem", sizeof(slab_t), NULL, NULL);
	slab_cache_mem = slab_create_cache("slab_cache_mem", sizeof(slab_cache_t), NULL, NULL);
}

slab_t *slab_create_slab(slab_cache_t *cache) {
	slab_t *slab = NULL;
	if (!slab_mem)
		slab = (slab_t*)vmm_alloc(kernel_pagemap, 1, MAP_READ | MAP_WRITE);
	else
		slab = (slab_t*)slab_alloc(slab_mem);
	slab->cache = cache;
	slab->magic = SLAB_MAGIC;
	slab->used_count = 0;
	slab->total_count = (PAGE_SIZE - 8) / (cache->obj_size + 8);
	// - 8 for slab poison at the end of the page, + 8 for free pointer poison.
	slab->buffer = vmm_alloc(kernel_pagemap, 1, MAP_READ | MAP_WRITE);
	// Set up free pointer poison.
	for (uint64_t i = 0; i < slab->total_count; i++) {
		uint64_t current_obj = (uint64_t)slab->buffer + i * (cache->obj_size + 8);
		if (i != slab->total_count - 1)
			*(uint64_t*)(current_obj) = current_obj + cache->obj_size + 8;
		else
			*(uint64_t*)(current_obj) = 0;
		if (cache->constructor) cache->constructor((void*)current_obj);
	}
	slab->next_free = (uint64_t)slab->buffer;
	*(uint64_t*)((uint64_t)slab->buffer + PAGE_SIZE - 8) = (uint64_t)slab;
	if (!cache->usable_slab_list) {
		cache->usable_slab_list = slab;
		slab->next = slab->prev = slab;
		return slab;
	}
	slab->next = cache->usable_slab_list;
	slab->prev = cache->usable_slab_list->prev;
	cache->usable_slab_list->prev->next = slab;
	cache->usable_slab_list->prev = slab;
	return slab;
}

slab_cache_t *slab_create_cache(const char *name, uint64_t obj_size,
	int (*constructor)(void *ptr), void (*destructor)(void *ptr)) {
	slab_cache_t *cache = NULL;
	if (!slab_cache_mem)
		cache = (slab_cache_t*)vmm_alloc(kernel_pagemap, 1, MAP_READ | MAP_WRITE);
	else
		cache = (slab_cache_t*)slab_alloc(slab_cache_mem);
	int name_len = strlen(name);
	name_len = name_len > 64 ? 64 : name_len;
	memcpy(cache->name, name, name_len);
	cache->obj_size = obj_size;
	cache->constructor = constructor;
	cache->destructor = destructor;
	cache->full_slab_list = NULL;
	slab_create_slab(cache);
	return cache;
}

void *slab_alloc(slab_cache_t *cache) {
	slab_t *slab = cache->usable_slab_list;
	uint64_t *free = (uint64_t*)slab->next_free;
	uint64_t next_free = *free;
	slab->next_free = next_free;
	slab->used_count++;

	void *obj = (void*)((uint64_t)free + 8);

	if (slab->used_count == slab->total_count) {
		cache->usable_slab_list = slab->next;
		if (cache->usable_slab_list == slab) {
			cache->usable_slab_list = NULL;
			slab_create_slab(cache);
		}
		if (!cache->full_slab_list) {
			cache->full_slab_list = slab;
			slab->next = slab->prev = slab;
		} else {
			slab->next = cache->full_slab_list;
			slab->prev = cache->full_slab_list->prev;
			cache->full_slab_list->prev->next = slab;
			cache->full_slab_list->prev = slab;
		}
	}

	return (void*)free;
}

void slab_free(void *ptr) {
	uint64_t slab_addr = ALIGN_DOWN((uint64_t)ptr, PAGE_SIZE) + PAGE_SIZE - 8;
	slab_t *slab = (slab_t*)(*(uint64_t*)slab_addr);
	if (slab->magic != SLAB_MAGIC) {
		kprintf(LOG_ERROR "Slab: Tried to free invalid pointer.\n");
		return;
	}
	slab_cache_t *cache = slab->cache;

	slab->used_count--;
	*(uint64_t*)((uint64_t)ptr - 8) = slab->next_free;
	slab->next_free = (uint64_t)ptr - 8;

	if (slab->used_count + 1 != slab->total_count)
		return;

	// Take it out of the used slab list.

	if (cache->full_slab_list->next == slab && cache->full_slab_list == slab)
		cache->full_slab_list = NULL;
	else {
		slab_t *new_head = cache->full_slab_list->next;
		new_head->prev = cache->full_slab_list->prev;
		if (new_head->next == slab) new_head->next = new_head;
		cache->full_slab_list = new_head;
	}

	if (!cache->usable_slab_list) {
		slab->next = slab->prev = slab;
		cache->usable_slab_list = slab;
		return;
	}
	slab->next = cache->usable_slab_list;
	slab->prev = cache->usable_slab_list->prev;
	cache->usable_slab_list->prev->next = slab;
	cache->usable_slab_list->prev = slab;
}

void slab_cache_info(slab_cache_t *cache) {
	kprintf(LOG_INFO "Slab (%s) info:\n", cache->name);
	kprintf(LOG_INFO " Object Size: %llu.\n", cache->obj_size);
	kprintf(LOG_INFO " Slabs:\n");
	slab_t *slab = cache->usable_slab_list;
	do {
		kprintf(LOG_INFO " -Usable Slab-\n");
		kprintf(LOG_INFO "  Total objects: %llu.\n", slab->total_count);
		kprintf(LOG_INFO "  Used objects: %llu.\n", slab->used_count);
		slab = slab->next;
	} while (slab != cache->usable_slab_list);
	slab = cache->full_slab_list;
	if (!slab) return;
	do {
		kprintf(LOG_INFO " -Full Slab-\n");
		kprintf(LOG_INFO "  Total objects: %llu.\n", slab->total_count);
		kprintf(LOG_INFO "  Used objects: %llu.\n", slab->used_count);
		slab = slab->next;
	} while (slab != cache->full_slab_list);
}
