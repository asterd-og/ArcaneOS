#include <arch/arch.h>
#include <arch/x86_64/acpi.h>
#include <arch/x86_64/madt.h>
#include <arch/x86_64/apic.h>
#include <arch/x86_64/pit.h>
#include <arch/x86_64/gdt.h>
#include <arch/interrupts.h>
#include <arch/smp.h>
#include <arch/mmu.h>
#include <kernel/kprintf.h>

void arch_early_init() {
	gdt_init(smp_get_bsp_id());
}

void arch_late_init() {
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
	cpu->id = mp_info->lapic_id;
	apic_timer_init();
	cpu->local_timer = timer_create(apic_timer_oneshot);
}

void arch_start_cpu(struct limine_mp_info *mp_info) {
	gdt_init(mp_info->lapic_id);
	interrupts_reload();
	mmu_load_pagemap(kernel_pagemap);
	cpu_t *cpu = smp_get_cpu(mp_info->lapic_id);
	cpu->id = mp_info->lapic_id;
	apic_init();
	apic_timer_init();
	cpu->local_timer = timer_create(apic_timer_oneshot);
}

uint64_t arch_this_cpu() {
	return (uint64_t)apic_get_id();
}
