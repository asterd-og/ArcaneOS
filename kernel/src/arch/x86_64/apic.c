#include <arch/x86_64/apic.h>
#include <arch/x86_64/madt.h>
#include <arch/x86_64/cpu.h>
#include <arch/x86_64/pit.h>
#include <arch/smp.h>
#include <kernel/kprintf.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

uint64_t apic_addr = 0;
bool x2apic_enabled = false;

void apic_init() {
	apic_addr = HIGHER_HALF(madt_apic_addr);
	mmu_map(kernel_pagemap, apic_addr, madt_apic_addr, MAP_READ | MAP_WRITE);

	uint64_t apic_flags = cpu_read_msr(APIC_MSR);
	apic_flags |= 0x800; // Enable apic

	uint32_t a = 1, b = 0, c = 0, d = 0;
	__asm__ volatile ("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(a));
	if (c & APIC_FLAG_X2APIC) {
		apic_flags |= 0x400; // Enable x2apic
		x2apic_enabled = true;
		kprintf(LOG_INFO "APIC: Using X2APIC.\n");
	}

	cpu_write_msr(APIC_MSR, apic_flags);

	uint64_t spurious_int = apic_read(APIC_REG_SPURIOUS_INT);
	spurious_int |= 0x100; // Enable it
	apic_write(APIC_REG_SPURIOUS_INT, spurious_int);
	kprintf(LOG_OK "APIC Initialised.\n");
}

void apic_eoi() {
	apic_write(APIC_REG_EOI, 0);
}

void apic_ipi(uint32_t id, uint32_t data, uint32_t type) {
	if (x2apic_enabled) {
		apic_write(APIC_REG_ICR_LO, (((uint64_t)id << 32) | data) | type);
		return;
	}
	apic_write(APIC_REG_ICR_HI, id << 24);
	apic_write(APIC_REG_ICR_LO, data | type);
}

uint32_t apic_get_id() {
	uint32_t id = apic_read(APIC_REG_ID);
	if (!x2apic_enabled) id >>= 24;
	return id;
}

void apic_timer_init() {
	apic_write(APIC_REG_DIV_CFG, 0x3);
	apic_write(APIC_REG_INIT_CNT, 0xffffffff);
	pit_wait(1); // Calibrate APIC timer to 1 ms.
	apic_write(APIC_REG_LVT_TIMER, 0x10000);
	uint64_t init_count = 0xffffffff - apic_read(APIC_REG_CURR_CNT);
	smp_this_cpu()->apic_timer_ticks = init_count;
}

void apic_timer_oneshot(timer_t *timer, uint64_t ms, uint8_t vec) {
	apic_write(APIC_REG_LVT_TIMER, 0x10000);
	apic_write(APIC_REG_INIT_CNT, 0);
	apic_write(APIC_REG_DIV_CFG, 0x3);
	apic_write(APIC_REG_INIT_CNT, ms * smp_this_cpu()->apic_timer_ticks);
	apic_write(APIC_REG_LVT_TIMER, vec);
}

void apic_write(uint32_t reg, uint64_t value) {
	if (x2apic_enabled) {
		reg = (reg >> 4) + 0x800;
		cpu_write_msr(reg, value);
		return;
	}
	uint64_t addr = apic_addr + reg;
	*((volatile uint32_t*)addr) = (uint32_t)value;
}

uint64_t apic_read(uint32_t reg) {
	if (x2apic_enabled) {
		reg = (reg >> 4) + 0x800;
		return cpu_read_msr(reg);
	}
	uint64_t addr = apic_addr + reg;
	return *((volatile uint32_t*)addr);
}

// I/O APIC

void ioapic_init() {
	// Map every I/O APIC to memory
	for (int i = 0; i < madt_ioapic_count; i++) {
		madt_ioapic_t *ioapic = madt_ioapic_vec[i];
		uint64_t addr = ioapic->ioapic_addr;
		mmu_map(kernel_pagemap, HIGHER_HALF(addr), addr, MAP_READ | MAP_WRITE);
	}
	kprintf(LOG_OK "I/O APIC Initialised.\n");
}

void ioapic_map_gsi(uint32_t apic_id, uint32_t gsi, uint8_t vec, uint32_t flags) {
	// Find I/O APIC for that GSI
	madt_ioapic_t *ioapic = NULL;
	for (int i = 0; i < madt_ioapic_count; i++) {
		ioapic = madt_ioapic_vec[i];
		if (i == madt_ioapic_count - 1) break;
		if (ioapic->gsi_base <= gsi)
			if (madt_ioapic_vec[i + 1]->gsi_base > gsi) break;
	}

	uint64_t data = (uint32_t)vec | flags;
	data |= (uint64_t)apic_id << 56;

	uint8_t reg = IOAPIC_REDIR_TABLE(gsi);

	ioapic_write(ioapic->ioapic_addr, reg, (uint32_t)data);
	ioapic_write(ioapic->ioapic_addr, reg + 1, (uint32_t)(data >> 32));
}

void ioapic_map_irq(uint32_t apic_id, uint8_t irq, uint8_t vec, bool mask) {
	madt_ioapic_iso_t *iso = NULL;
	for (int i = 0; i < madt_iso_count; i++)
		if (madt_iso_vec[i]->irq_src == irq) {
			iso = madt_iso_vec[i];
			break;
		}

	if (iso == NULL) {
		ioapic_map_gsi(apic_id, irq, vec, (mask ? 1 << 16 : 0));
		return;
	}

	uint32_t flags = 0;
	if (iso->flags & (1 << 1)) flags |= 1 << 13; // Polarity
	if (iso->flags & (1 << 3)) flags |= 1 << 15; // Trigger mode
	if (mask) flags |= (1 << 16);

	ioapic_map_gsi(apic_id, iso->gsi, vec, flags);
}

void ioapic_write(uint32_t base, uint8_t reg, uint32_t data) {
	uint64_t addr = HIGHER_HALF((uint64_t)base);
	*(volatile uint32_t*)addr = reg;
	*(volatile uint32_t*)(addr + 0x10) = data;
}

uint32_t ioapic_read(uint32_t base, uint8_t reg) {
	uint64_t addr = HIGHER_HALF((uint64_t)base);
	*(volatile uint32_t*)addr = reg;
	return *(volatile uint32_t*)(addr + 0x10);
}
