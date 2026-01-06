#include <fs/tmpfs.h>
#include <mm/slab.h>
#include <lib/string.h>
#include <limine.h>

__attribute__((used, section(".limine_requests")))
static volatile struct limine_module_request module_request = {
	.id = LIMINE_MODULE_REQUEST,
	.revision = 0
};

void tmpfs_init() {
	struct limine_module_response *module_response = module_request.response;
	struct limine_file *ramdisk = module_response->modules[0];

	vfs_t *tmpfs = arc_mount(ramdisk->address);
	strcpy(tmpfs->name, "tmpfs");

	vfs_register(tmpfs);
}

size_t arc_read(vnode_t *node, void *buf, uint64_t offset, uint64_t count) {
	uint64_t data_start = (uint64_t)node->metadata;

	memcpy(buf, (void*)data_start, count);

	return count;
}

vnode_t *arc_lookup(vnode_t *node, const char *name) {
	uint64_t arc_base = (uint64_t)node->vfs->metadata;
	arc_entry_t *file_table = (arc_entry_t*)node->metadata;

	int i = 0;

	while (memcmp(file_table[i].magic, "ARC", 3) == 0) {
		if (strcmp(file_table[i].name, name) != 0) { i++; continue; }

		// Make node.
		vnode_t *new_node = (vnode_t*)slab_alloc(node->vfs->cache);

		strcpy(new_node->name, file_table[i].name);
		new_node->size = file_table[i].size;
		new_node->ops = &node->vfs->ops;
		new_node->vfs = node->vfs;
		new_node->parent = node;
		new_node->child = NULL;
		new_node->next = new_node->prev = NULL;
		new_node->metadata = (void*)(file_table[i].data_off + arc_base);

		return new_node;
	}

	return NULL;
}

vfs_t *arc_mount(void *start) {
	vfs_t *vfs = slab_alloc(vfs_cache);

	vfs->cache = slab_create_cache("arc cache", sizeof(vnode_t), NULL, NULL);

	vfs->ops.read = arc_read;
	vfs->ops.write = NULL;
	vfs->ops.lookup = arc_lookup;

	vnode_t *root = (vnode_t*)slab_alloc(vfs->cache);
	root->ops = &vfs->ops;
	root->vfs = vfs;
	root->parent = NULL;
	root->child = NULL;
	root->next = root->prev = root;
	root->metadata = start;

	vfs->root = root;
	vfs->metadata = start;

	return vfs;
}
