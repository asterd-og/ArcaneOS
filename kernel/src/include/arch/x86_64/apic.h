#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define APIC_MSR 0x1B
#define APIC_REG_ID 0x20
#define APIC_REG_EOI 0xB0
#define APIC_REG_SPURIOUS_INT 0xF0
#define APIC_REG_ICR_LO 0x300
#define APIC_REG_ICR_HI 0x310
#define APIC_REG_LVT_TIMER 0x320
#define APIC_REG_INIT_CNT 0x380
#define APIC_REG_CURR_CNT 0x390
#define APIC_REG_DIV_CFG 0x3E0

#define APIC_IPI_SINGLE 0
#define APIC_IPI_EVERY 0x80000
#define APIC_IPI_OTHERS 0xC0000

#define APIC_FLAG_X2APIC (1 << 21)

void apic_init();
void apic_eoi();
void apic_ipi(uint32_t id, uint32_t data, uint32_t type);
uint32_t apic_get_id();
void apic_timer_init();
void apic_timer_oneshot(uint32_t vec, uint64_t ms);

void apic_write(uint32_t reg, uint64_t value);
uint64_t apic_read(uint32_t reg);

// I/O APIC

#define IOAPIC_REDIR_TABLE(n) (0x10 + 2 * n)

void ioapic_init();
void ioapic_map_irq(uint32_t apic_id, uint8_t irq, uint8_t vec, bool mask);

void ioapic_write(uint32_t base, uint8_t reg, uint32_t data);
uint32_t ioapic_read(uint32_t base, uint8_t reg);
