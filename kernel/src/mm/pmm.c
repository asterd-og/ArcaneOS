#include <mm/pmm.h>
#include <stddef.h>
#include <stdbool.h>
#include <lib/string.h>
#include <arch/mmu.h>
#include <limine.h>
#include <kernel/kprintf.h>

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
	.id = LIMINE_HHDM_REQUEST,
	.revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST,
	.revision = 0
};

uint64_t hhdm_offset = 0;
struct limine_memmap_response *memmap_response = NULL;

uint8_t *pmm_bitmap = NULL;
size_t pmm_bitmap_size = 0;
uint64_t pmm_last_free = 0;

uint8_t pmm_bitmap_get(uint64_t bit) {
	return (pmm_bitmap[bit / 8] & (1 << (bit % 8)));
}

void pmm_bitmap_set(uint64_t bit) {
	pmm_bitmap[bit / 8] |= (1 << (bit % 8));
}

void pmm_bitmap_clear(uint64_t bit) {
	pmm_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

void pmm_init() {
	hhdm_offset = hhdm_request.response->offset;
	memmap_response = memmap_request.response;

	struct limine_memmap_entry *memmap_entry = NULL;

	for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
		memmap_entry = memmap_response->entries[i];
		pmm_bitmap_size += memmap_entry->length;
	}

	bool found_entry = false;
	pmm_bitmap_size /= PAGE_SIZE; // Each bit is a page
	pmm_bitmap_size = ALIGN_UP(pmm_bitmap_size / 8, PAGE_SIZE);

	struct limine_memmap_entry *bitmap_entry = NULL;

	for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
		memmap_entry = memmap_response->entries[i];
		if (memmap_entry->type != LIMINE_MEMMAP_USABLE) continue;
		if (memmap_entry->length < pmm_bitmap_size) continue;
		found_entry = true;
		pmm_bitmap = (uint8_t*)HIGHER_HALF(memmap_entry->base);
		memset(pmm_bitmap, 0xFF, pmm_bitmap_size);
		bitmap_entry = memmap_entry;
		break;
	}

	if (!found_entry) {
		kprintf(LOG_ERROR "Could not find suitable entry in the memory map for the pmm bitmap.\n");
		return;
	}

	for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
		memmap_entry = memmap_response->entries[i];
		if (memmap_entry->type == LIMINE_MEMMAP_USABLE) {
			uint64_t base = memmap_entry->base;
			uint64_t length = memmap_entry->length;
			if (memmap_entry == bitmap_entry) {
				base += pmm_bitmap_size;
				length -= pmm_bitmap_size;
			}
			if (pmm_last_free == 0)
				pmm_last_free = ALIGN_DOWN(base, PAGE_SIZE) / PAGE_SIZE;
			for (uint64_t j = 0; j < length; j += PAGE_SIZE)
				pmm_bitmap_clear((base + j) / PAGE_SIZE);
		}
	}

	kprintf(LOG_OK "PMM Initialised.\n");
}

void *pmm_alloc() {
	uint64_t idx = pmm_last_free;
	while (pmm_bitmap_get(idx) != 0) {
		idx++;
	}
	pmm_last_free = idx;
	pmm_bitmap_set(idx);
	return (void*)(idx * PAGE_SIZE);
}

void pmm_free(void *page) {
	uint64_t bit = (uint64_t)page / PAGE_SIZE;
	pmm_bitmap_clear(bit);
}
