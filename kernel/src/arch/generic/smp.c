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

struct limine_mp_response *mp_response = NULL;

cpu_t *smp_cpu_vec[SMP_MAX_CPU_COUNT];

spinlock_t smp_lock = {0};

uint64_t smp_cpu_count = 1;
uint64_t smp_started_count = 1;
uint32_t smp_bsp_id = 0;
bool smp_enabled = false;

void smp_cpu_entry(struct limine_mp_info *mp_info) {
	arch_start_cpu(mp_info);
	spinlock_acquire(&smp_lock);
	smp_started_count++;
	spinlock_release(&smp_lock);
	while (1) arch_pause();
}

void smp_early_init() {
	mp_response = mp_request.response;
	smp_bsp_id = mp_response->bsp_lapic_id;
	smp_cpu_count = mp_response->cpu_count;
}

void smp_late_init() {
	struct limine_mp_response *mp_response = mp_request.response;
	memset(smp_cpu_vec, 0, SMP_MAX_CPU_COUNT * sizeof(cpu_t*));

	for (uint64_t i = 0; i < smp_cpu_count; i++) {
		struct limine_mp_info *mp_info = mp_response->cpus[i];
		cpu_t *cpu = (cpu_t*)alloc(sizeof(cpu_t));
		memset(cpu, 0, sizeof(cpu_t));
		cpu->interrupt_status = 1;
		smp_cpu_vec[mp_info->lapic_id] = cpu;
		if (mp_info->lapic_id == smp_bsp_id) {
			arch_setup_bsp(mp_info);
			continue;
		}
		cpu->thread_queue.list = list_create();
		cpu->thread_queue.current_item = cpu->thread_queue.list->head;
		mp_info->goto_address = smp_cpu_entry;
		mp_info->extra_argument = (uint64_t)mp_info;
	}

	while (smp_started_count < smp_cpu_count) __asm__ volatile ("pause");

	smp_enabled = true;
	kprintf(LOG_OK "SMP: Started %zu CPUs.\n", smp_cpu_count);
}

cpu_t *smp_get_cpu(uint64_t id) {
	if (id >= SMP_MAX_CPU_COUNT) return NULL;
	return smp_cpu_vec[id];
}

cpu_t *smp_this_cpu() {
	return smp_cpu_vec[arch_this_cpu()];
}
