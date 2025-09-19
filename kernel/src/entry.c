#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <drivers/fb.h>
#include <kernel/kprintf.h>
#include <lib/string.h>
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

typedef struct {
	char name[64];
	int test;
} foo_t;

void kmain() {

	framebuffer_early_init();
	pmm_init();
	vmm_init();
	slab_init();
	alloc_init();

	char *test = alloc(53);
	memcpy(test, "Hello world from generic allocator allocated string!\0", 53);
	kprintf("%s\n", test);
	free(test);
	kprintf(LOG_OK "OK!\n");

	while (1) ;
}
