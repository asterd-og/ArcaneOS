#pragma once

#include <stdint.h>
#include <arch/mmu.h>
#include <limine.h>

#define HIGHER_HALF(x) (typeof(x))((uint64_t)(x) + hhdm_offset)
#define LOWER_HALF(x) (typeof(x))((uint64_t)(x) - hhdm_offset)

#define DIV_ROUND_UP(x, y) (x + (y - 1)) / y
#define ALIGN_UP(x, y) DIV_ROUND_UP(x, y) * y
#define ALIGN_DOWN(x, y) ((x / y) * y)

extern uint64_t hhdm_offset;
extern struct limine_memmap_response *memmap_response;

void pmm_init();
void *pmm_alloc();
void pmm_free(void *page);
