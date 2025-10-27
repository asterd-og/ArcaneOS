#pragma once

#include <stdint.h>
#include <stddef.h>
#include <arch/context.h>
#include <mm/vmm.h>

typedef struct {
	uint64_t id;
	context_t ctx;
	pagemap_t *pagemap;
} thread_t;

thread_t *thread_create(void *entry);
