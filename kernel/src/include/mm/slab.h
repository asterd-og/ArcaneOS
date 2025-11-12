#pragma once

#include <stdint.h>
#include <stddef.h>
#include <lib/spinlock.h>

#define SLAB_MAGIC 0xDEADBEEFC0FFEEDD
#define SLAB_MAG_OBJS 32
#define SLAB_PAGE_COUNT 4

struct slab_cache;

typedef struct {
	uint64_t *objects[SLAB_MAG_OBJS];
	int count;
} magazine_t;

typedef struct slab {
	uint64_t magic;
	struct slab_cache *cache;
	struct slab *next;
	struct slab *prev;
	spinlock_t lock;
	uint64_t objs_per_page;
	uint64_t objs_per_mag;
	uint64_t total_count;
	uint64_t free_idx;
	uint16_t *free_objs; // In case there are no magazines to return it to.
	void *buffer;
} slab_t;

typedef struct slab_cache {
	char name[64];

	uint64_t obj_size;

	spinlock_t lock;

	int (*constructor)(void *ptr);
	void (*destructor)(void *ptr);

	slab_t *full_slab_list;
	slab_t *usable_slab_list;

	magazine_t *loaded_mag;
} slab_cache_t;

void slab_init();

slab_cache_t *slab_create_cache(const char *name, uint64_t obj_size, void *constructor, void *destructor);

void *slab_alloc(slab_cache_t *cache);
void slab_free(void *ptr);

void slab_cache_info(slab_cache_t *cache);
