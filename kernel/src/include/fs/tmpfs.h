#pragma once

#include <stdint.h>
#include <stddef.h>
#include <fs/vfs.h>

#define ARC_DIR 0
#define ARC_FILE 1

typedef struct {
	char magic[3];
	uint8_t type;
	char name[128];
	uint64_t size;
	uint64_t data_off;
} __attribute__((packed)) arc_entry_t;

void tmpfs_init();
vfs_t *arc_mount(void *start);
