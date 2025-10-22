#include <arch/smp.h>
#include <arch/arch.h>
#include <kernel/kprintf.h>
#include <lib/spinlock.h>
#include <lib/string.h>
#include <mm/alloc.h>
#include <limine.h>

__attribute__((used, section(".limine_requests")))
static volatile struct limine_mp_request mp_request = {
	.id = LIMINE_MP_REQUEST,
	.revision = 0
};

cpu_t *smp_cpu_vec[SMP_MAX_CPU_COUNT];

NEW_SPINLOCK(smp_lock);

uint64_t smp_cpu_count = 1;

void smp_cpu_entry(struct limine_mp_info *mp_info) {
	arch_start_cpu(mp_info);
	spinlock_acquire(&smp_lock);
	smp_cpu_count++;
	spinlock_release(&smp_lock);
	while (1) arch_pause();
}

void smp_init() {
	struct limine_mp_response *mp_response = mp_request.response;
	uint64_t expected_cpu_count = mp_response->cpu_count;
	memset(smp_cpu_vec, 0, SMP_MAX_CPU_COUNT * sizeof(cpu_t*));

	for (uint64_t i = 0; i < expected_cpu_count; i++) {
		struct limine_mp_info *mp_info = mp_response->cpus[i];
		cpu_t *cpu = (cpu_t*)alloc(sizeof(cpu_t));
		memset(cpu, 0, sizeof(cpu_t));
		cpu->interrupt_status = 1;
		smp_cpu_vec[mp_info->lapic_id] = cpu;
		if (mp_info->lapic_id == mp_response->bsp_lapic_id) {
			arch_setup_bsp(mp_info);
			continue;
		}
		mp_info->goto_address = smp_cpu_entry;
		mp_info->extra_argument = (uint64_t)mp_info;
	}

	while (smp_cpu_count < expected_cpu_count) __asm__ volatile ("pause");

	kprintf(LOG_OK "SMP: Started %zu CPUs.\n", smp_cpu_count);
}

cpu_t *smp_get_cpu(uint64_t id) {
	if (id >= SMP_MAX_CPU_COUNT) return NULL;
	return smp_cpu_vec[id];
}

cpu_t *smp_this_cpu() {
	return smp_cpu_vec[arch_this_cpu()];
}
