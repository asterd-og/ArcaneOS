#include <arch/arch.h>
#include <arch/x86_64/acpi.h>
#include <arch/x86_64/madt.h>
#include <arch/x86_64/apic.h>
#include <arch/x86_64/pit.h>
#include <arch/interrupts.h>
#include <arch/smp.h>
#include <arch/mmu.h>
#include <kernel/kprintf.h>

void arch_init() {
	acpi_init();
	madt_init();
	apic_init();
	ioapic_init();
	pit_init();
}

void arch_die() {
	__asm__ volatile ("cli");
	while (1) __asm__ volatile ("hlt");
}

void arch_setup_bsp(struct limine_mp_info *mp_info) {
	cpu_t *cpu = smp_get_cpu(mp_info->lapic_id);
	apic_timer_init();
}

void arch_start_cpu(struct limine_mp_info *mp_info) {
	interrupts_reload();
	mmu_load_pagemap(kernel_pagemap);
	cpu_t *cpu = smp_get_cpu(mp_info->lapic_id);
	apic_init();
	apic_timer_init();
}

uint64_t arch_this_cpu() {
	return (uint64_t)apic_get_id();
}
