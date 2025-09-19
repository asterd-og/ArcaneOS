#include <mm/vmm.h>
#include <mm/pmm.h>
#include <arch/mmu.h>
#include <lib/string.h>
#include <kernel/kprintf.h>

__attribute__((used, section(".limine_requests")))
static volatile struct limine_executable_address_request executable_address_request = {
	.id = LIMINE_EXECUTABLE_ADDRESS_REQUEST,
	.revision = 0
};

pagemap_t *kernel_pagemap = NULL;

extern char ld_text_start[];
extern char ld_text_end[];
extern char ld_rodata_start[];
extern char ld_rodata_end[];
extern char ld_data_start[];
extern char ld_data_end[];

void vmm_init() {
	kernel_pagemap = (pagemap_t*)HIGHER_HALF(pmm_alloc());
	kernel_pagemap->top_level = (uint64_t*)HIGHER_HALF(pmm_alloc());
	memset(kernel_pagemap->top_level, 0, PAGE_SIZE);
	vmm_setup_vma(kernel_pagemap, HIGHER_HALF(0x100000000000), 1);

	// Map kernel

	struct limine_executable_address_response *executable_address_response = executable_address_request.response;
	uint64_t virt_base = executable_address_response->virtual_base;
	uint64_t phys_base = executable_address_response->physical_base;

	uint64_t text_start = ALIGN_DOWN((uint64_t)ld_text_start, PAGE_SIZE);
	uint64_t text_end = ALIGN_UP((uint64_t)ld_text_end, PAGE_SIZE);
	uint64_t rodata_start = ALIGN_DOWN((uint64_t)ld_rodata_start, PAGE_SIZE);
	uint64_t rodata_end = ALIGN_UP((uint64_t)ld_rodata_end, PAGE_SIZE);
	uint64_t data_start = ALIGN_DOWN((uint64_t)ld_data_start, PAGE_SIZE);
	uint64_t data_end = ALIGN_UP((uint64_t)ld_data_end, PAGE_SIZE);

	for (uint64_t addr = text_start; addr < text_end; addr += PAGE_SIZE)
		mmu_map(kernel_pagemap, addr, addr - virt_base + phys_base, MAP_READ);
	for (uint64_t addr = rodata_start; addr < rodata_end; addr += PAGE_SIZE)
		mmu_map(kernel_pagemap, addr, addr - virt_base + phys_base, MAP_READ | MAP_NX);
	for (uint64_t addr = data_start; addr < data_end; addr += PAGE_SIZE)
		mmu_map(kernel_pagemap, addr, addr - virt_base + phys_base, MAP_READ | MAP_WRITE | MAP_NX);

	// Map memory map

	for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
		struct limine_memmap_entry *memmap_entry = memmap_response->entries[i];
		for (uint64_t j = 0; j < ALIGN_UP(memmap_entry->length, PAGE_SIZE); j += PAGE_SIZE) {
			uint64_t addr = memmap_entry->base + j;
			mmu_map(kernel_pagemap, HIGHER_HALF(addr), addr, MAP_READ | MAP_WRITE);
		}
	}

	mmu_load_pagemap(kernel_pagemap);

	kprintf(LOG_OK "Loaded kernel pagemap, VMM Initialised.\n");
}

vm_region_t *vmm_new_region(uint64_t base, uint64_t pages) {
	vm_region_t *region = (vm_region_t*)HIGHER_HALF(pmm_alloc());
	region->base = base;
	region->pages = pages;
	region->next = region->prev = NULL;
	return region;
}

void vmm_setup_vma(pagemap_t *pagemap, uint64_t base, uint64_t pages) {
	vm_region_t *region = vmm_new_region(base, pages);
	region->next = region->prev = region;
	pagemap->vm_region_head = region;
}

void vmm_insert(vm_region_t *prev, vm_region_t *region) {
	region->next = prev->next;
	region->prev = prev;
	prev->next->prev = region;
	prev->next = region;
}

void *vmm_alloc(pagemap_t *pagemap, uint64_t pages, uint64_t flags) {
	// Find a free region

	vm_region_t *region = pagemap->vm_region_head;
	for (; region->next != pagemap->vm_region_head; region = region->next) {
		vm_region_t *next = region->next;
		if (region->base + region->pages * PAGE_SIZE == next->base)
			continue;
		uint64_t gap_base = region->base + region->pages * PAGE_SIZE;
		uint64_t gap_length = next->base - gap_base;
		if (gap_length < pages * PAGE_SIZE)
			continue;
		kprintf(LOG_INFO "VMM: Found a hole between 0x%p and 0x%p.\n",
			region->base + region->pages * PAGE_SIZE, next->base);
		break;
	}

	uint64_t base = region->base + region->pages * PAGE_SIZE;

	vm_region_t *new_region = vmm_new_region(base, pages);
	vmm_insert(region, new_region);

	// Allocate and map the physical memory

	for (uint64_t i = 0; i < pages; i++)
		mmu_map(pagemap, base + i * PAGE_SIZE, (uint64_t)pmm_alloc(), flags);

	return (void*)base;
}

void vmm_free(pagemap_t *pagemap, void *ptr) {
	uint64_t base = (uint64_t)ptr;

	vm_region_t *region = pagemap->vm_region_head->next;
	for (; region != pagemap->vm_region_head; region = region->next) {
		if (region->base == base) break;
	}

	if (region == pagemap->vm_region_head)
		return;

	for (uint64_t i = 0; i < region->pages; i++) {
		uint64_t page = mmu_get_page(pagemap, base + i * PAGE_SIZE);
		if (page == 0) {
			kprintf(LOG_ERROR "VMM: Trying to free a non-existent page.\n");
			return;
		}
		pmm_free((void*)GET_ADDR_FROM_PAGE(page));
	}

	region->prev->next = region->next;
	region->next->prev = region->prev;
	pmm_free(LOWER_HALF(region));

	kprintf(LOG_OK "VMM: Freed 0x%p.\n", base);
}
