#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
	char name[100];
	uint64_t mode;
	uint64_t owner;
	uint64_t group;
	char size[12];
	char mod_time[12];
	uint64_t checksum;
	uint8_t type;
	char linked_name[100];
	char indicator[6];
	char version[2];
	char owner_name[32];
	char group_name[32];
	uint64_t dev_major;
	uint64_t dev_minor;
	char prefix[155];
} ustar_file_t;
