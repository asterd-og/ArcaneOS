#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct list_item {
	struct list_item *next;
	struct list_item *prev;
	void *data;
} list_item_t;

typedef struct {
	list_item_t *head;
	uint64_t count;
} list_t;

list_t *list_create();
list_item_t *list_insert(list_t *list, void *data);
