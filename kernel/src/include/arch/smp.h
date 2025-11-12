#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <arch/context.h>
#include <lib/spinlock.h>
#include <lib/list.h>
#include <sys/timer.h>
#include <sys/thread.h>

#if defined(__x86_64__)
#include <arch/x86_64/apic.h>
#endif

#define SMP_MAX_CPU_COUNT 256

typedef struct {
	list_t *list;
	list_item_t *current_item;
} thread_queue_t;

typedef struct {
	uint32_t id;
	int interrupt_status;
	timer_t *local_timer;
	context_t *trap_frame;

	thread_queue_t thread_queue;
	thread_t *current_thread;

#if defined(__x86_64__)
	uint32_t apic_timer_ticks;
#endif
} cpu_t;

extern uint32_t smp_bsp_id;
extern uint64_t smp_cpu_count;
extern bool smp_enabled;

void smp_early_init();
void smp_late_init();
uint32_t smp_get_bsp_id();
cpu_t *smp_get_cpu(uint64_t id);
cpu_t *smp_this_cpu();

static inline void smp_ipi(uint32_t id, uint8_t vec) {
#if defined (__x86_64__)
	apic_ipi(id, vec, APIC_IPI_SINGLE);
#endif
}

static inline void smp_ipi_others(uint8_t vec) {
#if defined (__x86_64__)
	apic_ipi(0, vec, APIC_IPI_OTHERS);
#endif
}
