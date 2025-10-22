#pragma once

#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/acpi.h>

typedef struct {
	sdt_header_t sdt_header;

	uint32_t apic_addr;
	uint32_t flags;

	char entry_table[];
} madt_t;

typedef struct {
	uint16_t resv;

	uint8_t ioapic_id;
	uint8_t resv1;
	uint32_t ioapic_addr;
	uint32_t gsi_base;
} madt_ioapic_t;

typedef struct {
	uint16_t resv;

	uint8_t bus_src;
	uint8_t irq_src;
	uint32_t gsi;
	uint16_t flags;
} madt_ioapic_iso_t;

typedef struct {
	uint16_t resv;

	uint16_t resv1;
	uint64_t lapic_addr;
} madt_lapic_override_t;

extern uint64_t madt_apic_addr;

extern madt_ioapic_t *madt_ioapic_vec[32];
extern madt_ioapic_iso_t *madt_iso_vec[32];

extern int madt_ioapic_count;
extern int madt_iso_count;

void madt_init();
