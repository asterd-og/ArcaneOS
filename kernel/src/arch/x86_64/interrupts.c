#include <arch/interrupts.h>
#include <arch/arch.h>
#include <arch/smp.h>
#include <arch/x86_64/apic.h>
#include <arch/x86_64/idt.h>
#include <kernel/kprintf.h>
#include <kernel/assert.h>

__attribute__((aligned(16))) static idt_entry_t idt_entries[256];
static idt_desc_t idt_register;
extern void *isr_table[256];
void *idt_handlers[256] = { 0 };

void interrupts_set_entry(uint16_t vector, void *isr, uint8_t flags) {
	idt_entry_t *entry = &idt_entries[vector];
	entry->off_low = (uint16_t)((uint64_t)isr & 0xFFFF);
	entry->selector = 0x08;
	entry->ist = 0;
	entry->flags = flags;
	entry->off_mid = (uint16_t)(((uint64_t)isr >> 16) & 0xFFFF);
	entry->off_high = (uint32_t)(((uint64_t)isr >> 32) & 0xFFFFFFFF);
	entry->zero = 0;
}

void interrupts_init() {
	for (uint16_t vector = 0; vector < 256; vector++)
		interrupts_set_entry(vector, isr_table[vector], 0x8E);

	idt_register.size = sizeof(idt_entries) - 1;
	idt_register.addr = (uint64_t)&idt_entries;

	__asm__ volatile ("lidt %0" : : "m"(idt_register) : "memory");
	__asm__ volatile ("sti");
}

void interrupts_reload() {
	__asm__ volatile ("lidt %0" : : "m"(idt_register) : "memory");
	__asm__ volatile ("sti");
}

int interrupts_set(int state) {
	cpu_t *this_cpu = smp_this_cpu();
	int old_state = this_cpu->interrupt_status;
	this_cpu->interrupt_status = state;
	if (state) __asm__ volatile ("sti");
	else __asm__ volatile ("cli");
	return old_state;
}

void interrupts_set_handler(uint8_t vector, void *handler) {
	idt_handlers[vector] = handler;
	if (vector >= 32 && vector < 48)
		ioapic_map_irq(0, vector - 32, vector, false);
}

uint8_t interrupts_alloc_vec() {
	static uint8_t free_vector = 48;
	ASSERT((free_vector < 255) && "No free vectors left");
	return free_vector++;
}

void interrupts_handle_int(context_t *ctx) {
	void(*handler)(context_t*) = idt_handlers[ctx->int_no];

	if (handler) {
		if (smp_enabled) smp_this_cpu()->trap_frame = ctx;
		handler(ctx);
		return;
	}

	kprintf(LOG_ERROR "Interrupts: Unhandled interrupt %d.\n", ctx->int_no);
	arch_die();
}

void interrupts_eoi() {
	apic_eoi();
}
