#pragma once

#include <stdint.h>
#include <stddef.h>
#include <sys/thread.h>
#include <arch/smp.h>

extern uint8_t sched_vec;

void sched_init();
void sched_start();
void sched_cpu_add_thread(cpu_t *cpu, thread_t *thread);
void sched_add_thread(thread_t *thread);
