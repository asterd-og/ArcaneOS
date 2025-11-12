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
#include <sys/scheduler.h>
#include <sys/thread.h>
#include <sys/timer.h>

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

void thread_a() {
	kprintf("Hello from this kernel's first ever thread! (after idle)\n");
	while (1);
}

void thread_b() {
	kprintf("Hello from the second thread! (Should be in another CPU!)\n");
	while (1);
}

void kmain() {
	framebuffer_early_init();
	smp_early_init();
	arch_early_init();
	interrupts_init();
	pmm_init();
	vmm_init();
	slab_init();
	alloc_init();
	arch_late_init();
	smp_late_init();
	sched_init();

	thread_t *thread = thread_create(thread_a);
	thread_t *thread1 = thread_create(thread_b);
	sched_add_thread(thread);
	sched_add_thread(thread1);

	sched_start();

	while (1) ;
}
