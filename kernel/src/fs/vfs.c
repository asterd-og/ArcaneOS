#include <fs/vfs.h>
#include <lib/string.h>
#include <kernel/kprintf.h>
#include <mm/alloc.h>

slab_cache_t *vfs_cache = NULL;
vfs_t *vfs_head = NULL;

void vfs_init() {
	vfs_cache = slab_create_cache("vfs cache", sizeof(vfs_t), NULL, NULL);
}

void vfs_register(vfs_t *vfs) {
	if (vfs_head == NULL) {
		vfs->next = NULL;
		vfs_head = vfs;

		return;
	}

	vfs_t *current = vfs_head;

	while (current->next != NULL) current = current->next;

	current->next = vfs;
}

vnode_t *vfs_find(const char *path) {
	// Path style: mount:/...

	char *temp = (char*)alloc(strlen(path) + 1);
	strcpy(temp, path);
	
	size_t mnt_lim = strcspn(temp, ":");
	temp[mnt_lim] = 0;

	vfs_t *vfs = vfs_head;
	while (strcmp(vfs->name, temp) != 0) {
		vfs = vfs->next;

		if (vfs == vfs_head) {
			free(temp);
			return NULL;
		}
	}

	vnode_t *current = vfs->root;

	size_t path_off = mnt_lim + 2;

	while (1) {
		if (temp[path_off] == 0) break;

		size_t name_lim = strcspn(temp + path_off, "/");
		temp[path_off + name_lim] = 0;

		kprintf("Looking up %s\n", temp + path_off);

		current = vfs_lookup(current, temp + path_off);

		if (current == NULL) {
			free(temp);
			return NULL;
		}

		path_off += name_lim + 1;
	}

	return current;
}

size_t vfs_read(vnode_t *node, void *buf, uint64_t offset, uint64_t count) {
	if (node->ops->read == NULL) return 0;
	size_t res = node->ops->read(node, buf, offset, count);
	return res;
}

size_t vfs_write(vnode_t *node, void *buf, uint64_t offset, uint64_t count) {
	if (node->ops->write == NULL) return 0;
	size_t res = node->ops->write(node, buf, offset, count);
	return res;
}

vnode_t *vfs_lookup(vnode_t *node, const char *name) {
	if (node->ops->lookup == NULL) return 0;
	vnode_t *res = node->ops->lookup(node, name);
	return res;
}
