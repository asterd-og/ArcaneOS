#pragma once

#include <stdint.h>

typedef struct vm_region {
	uint64_t base;
	uint64_t pages;
	struct vm_region *prev;
	struct vm_region *next;
} vm_region_t;

typedef struct {
	uint64_t *top_level;
	vm_region_t *vm_region_head;
} pagemap_t;

extern pagemap_t *kernel_pagemap;

void vmm_init();

pagemap_t *vmm_new_pagemap();

void vmm_setup_vma(pagemap_t *pagemap, uint64_t base, uint64_t pages);
void *vmm_alloc(pagemap_t *pagemap, uint64_t pages, uint64_t flags);
void vmm_free(pagemap_t *pagemap, void *ptr);
