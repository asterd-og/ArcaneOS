#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
	uint16_t off_low;
	uint16_t selector;
	uint8_t ist;
	uint8_t flags;
	uint16_t off_mid;
	uint32_t off_high;
	uint32_t zero;
} __attribute__((packed)) idt_entry_t;

typedef struct {
	uint16_t size;
	uint64_t addr;
} __attribute__((packed)) idt_desc_t;
