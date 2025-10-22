#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <drivers/fb.h>
#include <kernel/kprintf.h>
#include <kernel/assert.h>
#include <lib/string.h>
#include <arch/interrupts.h>
#include <arch/arch.h>
#include <arch/smp.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/slab.h>
#include <mm/alloc.h>

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

void kmain() {
	framebuffer_early_init();
	interrupts_init();
	pmm_init();
	vmm_init();
	slab_init();
	alloc_init();
	arch_init();
	smp_init();

	while (1) ;
}
