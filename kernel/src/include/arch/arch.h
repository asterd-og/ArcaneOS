#pragma once

#include <limine.h>

void arch_early_init();
void arch_late_init();
void arch_die();
void arch_setup_bsp(struct limine_mp_info *mp_info);
void arch_start_cpu(struct limine_mp_info *mp_info);
uint64_t arch_this_cpu();

#if defined(__x86_64__)
static inline void arch_pause() {
	__asm__ volatile ("pause");
}
#endif
