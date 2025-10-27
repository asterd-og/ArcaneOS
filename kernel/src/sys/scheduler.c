#include <sys/scheduler.h>
#include <arch/interrupts.h>
#include <arch/smp.h>
#include <arch/mmu.h>
#include <kernel/kprintf.h>

uint8_t sched_vec = 0;

void sched_preempt();

void sched_idle() {
	while (1);
}

void sched_init() {
	sched_vec = interrupts_alloc_vec();
	interrupts_set_handler(sched_vec, sched_preempt);

	// Create idle threads
	for (int i = 0; i < smp_cpu_count; i++) {
		cpu_t *cpu = smp_get_cpu(i);
		if (cpu->id == smp_get_bsp_id()) continue;
		thread_t *idle_thread = thread_create(sched_idle);
		sched_cpu_add_thread(smp_get_cpu(i), idle_thread);
	}
}

void sched_start() {
	smp_ipi_others(sched_vec);
}

cpu_t *sched_pick_cpu() {
	// Pick CPU with the least load
	cpu_t *best_pick = NULL;
	for (int i = 0; i < smp_cpu_count; i++) {
		cpu_t *cpu = smp_get_cpu(i);
		if (cpu->id == smp_get_bsp_id()) continue;
		if (best_pick == NULL) {
			best_pick = cpu;
			continue;
		}
		if (cpu->thread_queue.list->count < best_pick->thread_queue.list->count)
			best_pick = cpu;
	}
	return best_pick;
}

void sched_cpu_add_thread(cpu_t *cpu, thread_t *thread) {
	list_insert(cpu->thread_queue.list, thread);
}

void sched_add_thread(thread_t *thread) {
	cpu_t *cpu = sched_pick_cpu();
	list_insert(cpu->thread_queue.list, thread);
	kprintf("Added thread to CPU %d\n", cpu->id);
}

void sched_switch_thread(thread_t *thread) {
	cpu_t *this_cpu = smp_this_cpu();

	if (this_cpu->current_thread)
		this_cpu->current_thread->ctx = *this_cpu->trap_frame;

	mmu_load_pagemap(thread->pagemap);
	*this_cpu->trap_frame = thread->ctx;
	this_cpu->current_thread = thread;
}

void sched_preempt() {
	// Pick next thread and switch to it!
	cpu_t *this_cpu = smp_this_cpu();

	list_item_t *queue_item = this_cpu->thread_queue.current_item;
	if (queue_item == this_cpu->thread_queue.list->head)
		queue_item = this_cpu->thread_queue.list->head->next;

	thread_t *next_thread = (thread_t*)queue_item->data;
	this_cpu->thread_queue.current_item = queue_item->next;

	sched_switch_thread(next_thread);

	this_cpu->local_timer->oneshot(this_cpu->local_timer, 5, sched_vec);
	interrupts_eoi();
}
