#include <mm/slab.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/alloc.h>
#include <arch/mmu.h>
#include <arch/smp.h>
#include <arch/arch.h>
#include <lib/string.h>
#include <lib/math.h>
#include <kernel/kprintf.h>
#include <kernel/assert.h>
#include <stddef.h>

#define SLAB_OBJ(slab, idx) \
	(void*)((uint64_t)slab->buffer + \
					(((uint64_t)(idx) / slab->objs_per_page) * PAGE_SIZE) +		\
					(((uint64_t)(idx) % slab->objs_per_page) * slab->cache->obj_size))

slab_cache_t *slab_mem = NULL;
slab_cache_t *slab_cache_mem = NULL;

void slab_init() {
	slab_mem = slab_create_cache("slab_mem", sizeof(slab_t), NULL, NULL);
	slab_cache_mem = slab_create_cache("cache_mem", sizeof(slab_cache_t), NULL, NULL);

	kprintf(LOG_OK "SLAB Initialised.\n");
}

void slab_load_mag(slab_t *slab, magazine_t *mag, uint64_t obj_count) {
	for (uint64_t i = mag->count; i < obj_count; i++) {
		mag->objects[i] = (uint64_t*)SLAB_OBJ(slab, slab->free_objs[slab->free_idx]);
		slab->free_idx--;
	}

	mag->count += obj_count;
}

void slab_setup_mag(slab_cache_t *cache, slab_t *slab) {
	for (uint64_t i = 0; i < smp_cpu_count; i++) {
		magazine_t *mag = &cache->loaded_mag[i];

		uint64_t obj_count = clamp(slab->free_idx, 0, slab->objs_per_mag);
		if (obj_count == 0) break; // No more objects to add to any other mags.

		slab_load_mag(slab, mag, obj_count);
	}
}

slab_t *slab_remove_from(slab_t **list, slab_t *slab) {
	slab_cache_t *cache = slab->cache;

	spinlock_acquire(&cache->lock);

	slab->next->prev = slab->prev;
	slab->prev->next = slab->next;

	if (*list == slab)
		*list = (slab->next == slab ? NULL : slab->next);

	slab->next = slab->prev = NULL;

	spinlock_release(&cache->lock);
	
	return slab;
}

slab_t *slab_add_to(slab_t **list, slab_t *slab) {
	slab_cache_t *cache = slab->cache;

	spinlock_acquire(&cache->lock);

	if (*list == NULL) {
		*list = slab;
		slab->next = slab->prev = slab; // Expects slab to not be in any other list.

		spinlock_release(&cache->lock);

		return slab;
	}

	slab->next = *list;
	slab->prev = (*list)->prev;
	(*list)->prev->next = slab;
	(*list)->prev = slab;

	spinlock_release(&cache->lock);

	return slab;
}

slab_t *slab_create_slab(slab_cache_t *cache) {
	slab_t *slab = NULL;
	if (slab_mem == NULL)
		slab = (slab_t*)vmm_alloc(kernel_pagemap, 1, MAP_READ | MAP_WRITE);
	else
		slab = (slab_t*)slab_alloc(slab_mem);

	slab->cache = cache;
	slab->magic = SLAB_MAGIC;

	slab->lock = (spinlock_t){0};

	uint64_t objs_per_page = (PAGE_SIZE - 8) / cache->obj_size;
	slab->total_count = objs_per_page * SLAB_PAGE_COUNT;
	slab->objs_per_page = objs_per_page;
	slab->objs_per_mag = clamp(slab->total_count / 2, 1, 32);

	slab->buffer = vmm_alloc(kernel_pagemap, SLAB_PAGE_COUNT, MAP_READ | MAP_WRITE);

	// Set up free objs.
	slab->free_objs = (uint16_t*)alloc(2 * slab->total_count);
	slab->free_idx = slab->total_count - 1;

	for (uint64_t i = 0; i < slab->total_count; i++)
		slab->free_objs[i] = (uint16_t)i; // Usually we wont go past 65535 objects per slab.

	// Construct objects.
	if (cache->constructor) {
		uint64_t current_obj = 0;
		for (uint64_t i = 0; i < slab->total_count; i++) {
			void *obj = SLAB_OBJ(slab, current_obj);
			cache->constructor(obj);
			current_obj++;
		}
	}

	// Set up slab poison.
	for (uint64_t i = 0; i < SLAB_PAGE_COUNT; i++) {
		uint64_t current_page = (uint64_t)slab->buffer + PAGE_SIZE * i;
		*(uint64_t*)(current_page + PAGE_SIZE - 8) = (uint64_t)slab;
	}

	slab_add_to(&cache->usable_slab_list, slab);

	return slab;
}

slab_cache_t *slab_create_cache(const char *name, uint64_t obj_size, void *constructor, void *destructor) {
	slab_cache_t *cache = NULL;
	if (slab_cache_mem == NULL)
		cache = (slab_cache_t*)vmm_alloc(kernel_pagemap, 1, MAP_READ | MAP_WRITE);
	else
		cache = (slab_cache_t*)slab_alloc(slab_cache_mem);

	int name_len = strlen(name);
	name_len = name_len > 64 ? 64 : name_len;

	memcpy(cache->name, name, name_len);

	cache->obj_size = obj_size;

	cache->lock = (spinlock_t){0};

	cache->constructor = constructor;
	cache->destructor = destructor;

	cache->full_slab_list = NULL;
	cache->usable_slab_list = NULL;

	cache->loaded_mag = (magazine_t*)alloc(sizeof(magazine_t) * smp_cpu_count);

	slab_t *first_slab = slab_create_slab(cache);
	slab_setup_mag(cache, first_slab);

	return cache;
}

void *slab_alloc(slab_cache_t *cache) {
	magazine_t *mag = &cache->loaded_mag[smp_bsp_id];

	if (smp_enabled)
		mag = &cache->loaded_mag[arch_this_cpu()];

	if (mag->count > 0)
		return (void*)mag->objects[--mag->count];

	// Load rounds from global object pool, needs locking.
	slab_t *slab = cache->usable_slab_list;

	spinlock_acquire(&slab->lock);

	uint64_t count = slab->objs_per_mag;

	if (slab->free_idx < count) {
		// If there aren't enough objects to fill the mag
		// then load what we have and create another slab.
		count = slab->free_idx;
		slab_load_mag(slab, mag, count);
		count = slab->objs_per_mag - count;

		slab_remove_from(&cache->usable_slab_list, slab);

		slab = cache->usable_slab_list;
		if (slab == NULL)
			slab = slab_create_slab(cache);
	}

	slab_load_mag(slab, mag, count);

	spinlock_release(&slab->lock);

	return (void*)mag->objects[--mag->count];
}

void slab_free(void *ptr) {
	uint64_t slab_addr = ALIGN_DOWN((uint64_t)ptr, PAGE_SIZE) + PAGE_SIZE - 8;
	slab_t *slab = (slab_t*)(*(uint64_t*)slab_addr);

	ASSERT(slab->magic == SLAB_MAGIC && "SLAB: Tried to free invalid ptr.");

	// Try to insert it back into a magazine.
	slab_cache_t *cache = slab->cache;

	ASSERT(((uint64_t)ptr % cache->obj_size) == 0 && "SLAB: Tried to free an invalid ptr.");

	magazine_t *mag = cache->loaded_mag;

	if (mag->count < slab->objs_per_mag) {
		mag->objects[mag->count++] = (uint64_t*)ptr;
		return;
	}

	kprintf(LOG_WARN "SLAB: TODO add object back to a slab, magazine is full.\n");
}
