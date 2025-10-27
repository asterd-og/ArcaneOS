#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
	uint16_t len;
	uint16_t base;
	uint8_t base1;
	uint8_t flags;
	uint8_t flags1;
	uint8_t base2;
	uint32_t base3;
	uint32_t resv;
} __attribute__((packed)) tss_entry_t;

typedef struct {
	uint32_t resv;
	uint64_t rsp[3];
	uint64_t resv1;
	uint64_t ist[7];
	uint64_t resv2;
	uint16_t resv3;
	uint16_t iopb;
} __attribute__((packed)) tss_desc_t;

typedef struct {
	uint64_t entries[5];
	tss_entry_t tss_entry;
} __attribute__((packed)) gdt_table_t;

typedef struct {
	uint16_t size;
	uint64_t addr;
} __attribute__((packed)) gdt_desc_t;

void gdt_init(uint32_t cpu_id);
void tss_set_rsp(uint32_t cpu_id, int rsp, void *addr);
