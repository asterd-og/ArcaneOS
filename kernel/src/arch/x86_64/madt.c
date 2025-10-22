#include <arch/x86_64/madt.h>
#include <kernel/kprintf.h>
#include <kernel/assert.h>
#include <mm/pmm.h>

madt_ioapic_t *madt_ioapic_vec[32] = { 0 };
madt_ioapic_iso_t *madt_iso_vec[32] = { 0 };

int madt_ioapic_count = 0;
int madt_iso_count = 0;

uint64_t madt_apic_addr = 0;

void madt_init() {
	madt_t *madt = (madt_t*)acpi_find_table("APIC");
	ASSERT((LOWER_HALF(madt) != NULL) && "Could not find APIC table in ACPI.");

	madt_apic_addr = madt->apic_addr;

	uint64_t offset = 0;
	uint64_t table_size = madt->sdt_header.len - sizeof(madt_t);
	while (offset < table_size) {
		uint8_t entry_type = madt->entry_table[offset];
		uint8_t entry_len = madt->entry_table[offset + 1];
		void *entry_data = (void*)(madt->entry_table + offset);
		switch (entry_type) {
		case 1: {
			madt_ioapic_t *ioapic = (madt_ioapic_t*)entry_data;
			madt_ioapic_vec[madt_ioapic_count++] = ioapic;
			break;
		}
		case 2: {
			madt_ioapic_iso_t *iso = (madt_ioapic_iso_t*)entry_data;
			madt_iso_vec[madt_iso_count++] = iso;
			kprintf(LOG_INFO "Found Interrupt Source Override for IRQ #%d.\n", iso->irq_src);
			break;
		}
		case 5: {
			madt_lapic_override_t *lapic_override = (madt_lapic_override_t*)entry_data;
			madt_apic_addr = lapic_override->lapic_addr;
			break;
		}
		}
		offset += entry_len;
	}
	kprintf(LOG_INFO "MADT Found %d I/O APICs.\n", madt_ioapic_count);
}
