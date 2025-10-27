#include <lib/list.h>
#include <mm/alloc.h>

list_t *list_create() {
	list_t *list = (list_t*)alloc(sizeof(list_t));
	list->head = (list_item_t*)alloc(sizeof(list_item_t));
	list->head->next = list->head->prev = list->head;
	list->count = 0;
	return list;
}

list_item_t *list_insert(list_t *list, void *data) {
	list_item_t *item = (list_item_t*)alloc(sizeof(list_item_t));
	item->next = list->head;
	item->prev = list->head->prev;
	list->head->prev->next = item;
	list->head->prev = item;
	item->data = data;
	list->count++;
	return item;
}
