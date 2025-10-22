#include <arch/x86_64/acpi.h>
#include <stdbool.h>
#include <limine.h>
#include <kernel/kprintf.h>
#include <kernel/assert.h>
#include <mm/pmm.h>
#include <lib/string.h>

__attribute__((used, section(".limine_requests")))
static volatile struct limine_rsdp_request rsdp_request = {
	.id = LIMINE_RSDP_REQUEST,
	.revision = 0
};

void *sdt_address = NULL;
bool use_xsdt = false;

void acpi_init() {
	struct limine_rsdp_response *rsdp_response = rsdp_request.response;
	xsdp_t *rsdp = (xsdp_t*)HIGHER_HALF(rsdp_response->address);
	ASSERT(!memcmp(rsdp->sign, "RSD PTR", 7) && "Invalid ACPI signature.");
	use_xsdt = rsdp->rev == 2;
	sdt_address = use_xsdt ? (void*)rsdp->xsdt_addr : (void*)rsdp->rsdt_addr;
	sdt_address = HIGHER_HALF(sdt_address);
}

void *acpi_find_table(const char *sign) {
	sdt_header_t *sdt_header = (sdt_header_t*)sdt_address;
	size_t entry_size = use_xsdt ? 8 : 4;
	int entry_count = (sdt_header->len - sizeof(sdt_header_t)) / entry_size;
	uint64_t *table_start = (uint64_t*)(sdt_header + 1);
	for (int i = 0; i < entry_count; i++) {
		sdt_header_t *header;
		uint64_t address = use_xsdt ? *(table_start + i) : *((uint32_t*)table_start + i);
		header = (sdt_header_t*)HIGHER_HALF(address);
		if (!memcmp(header->sign, sign, 4))
			return (void*)header;
	}
	return NULL;
}
