#include <sys/thread.h>
#include <mm/alloc.h>
#include <mm/vmm.h>
#include <arch/mmu.h>

uint64_t thread_global_id = 0;

thread_t *thread_create(void *entry) {
	thread_t *thread = (thread_t*)alloc(sizeof(thread_t));
	thread->id = thread_global_id++;
	thread->pagemap = vmm_new_pagemap();
	CTX_IP(thread->ctx) = (uint64_t)entry;
	// TODO: Start allocating in the thread's pagemap.
	CTX_SP(thread->ctx) = (uint64_t)vmm_alloc(kernel_pagemap, 4, MAP_READ | MAP_WRITE | MAP_USER);
	thread->ctx.rflags = 0x202;
	thread->ctx.cs = 0x08;
	thread->ctx.ss = 0x10;
	return thread;
}
