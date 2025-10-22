#pragma once

#include <stdint.h>
#include <stddef.h>
#include <lib/spinlock.h>

#define SMP_MAX_CPU_COUNT 256

typedef struct {
	int interrupt_status;
#if defined(__x86_64__)
	uint32_t apic_timer_ticks;
#endif
} cpu_t;

extern uint64_t smp_cpu_count;

void smp_init();
cpu_t *smp_get_cpu(uint64_t id);
cpu_t *smp_this_cpu();
