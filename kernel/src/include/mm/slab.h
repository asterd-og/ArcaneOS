#pragma once

#include <stdint.h>

#define SLAB_MAGIC 0xDEADBEEFC0FFEEDD

struct slab_cache;

typedef struct slab {
	uint64_t magic;
	struct slab_cache *cache;
	struct slab *next;
	struct slab *prev;
	uint64_t used_count;
	uint64_t total_count;
	uint64_t next_free; // Use free pointer poisoning.
	void *buffer;
} slab_t;

typedef struct slab_cache {
	char name[64];

	uint64_t obj_size;

	int (*constructor)(void *ptr);
	void (*destructor)(void *ptr);

	slab_t *full_slab_list;
	slab_t *usable_slab_list;
} slab_cache_t;

void slab_init();

slab_cache_t *slab_create_cache(const char *name, uint64_t obj_size,
	int (*constructor)(void *ptr), void (*destructor)(void *ptr));

void *slab_alloc(slab_cache_t *cache);
void slab_free(void *ptr);

void slab_cache_info(slab_cache_t *cache);
