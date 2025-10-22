#pragma once

#include <stdint.h>

typedef struct {
	char sign[8];
	uint8_t checksum;
	char oem[6];
	uint8_t rev;
	uint32_t rsdt_addr;
	uint32_t len;
	uint64_t xsdt_addr;
	uint8_t ext_checksum;
	uint8_t resv[8];
} xsdp_t;

typedef struct {
	char sign[4];
	uint32_t len;
	uint8_t rev;
	uint8_t checksum;
	char oem[6];
	char oem_table[8];
	uint32_t oem_rev;
	uint32_t creator_id;
	uint32_t creator_rev;
} sdt_header_t;

void acpi_init();
void *acpi_find_table(const char *sign);
