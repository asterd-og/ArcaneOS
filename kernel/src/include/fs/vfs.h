#pragma once

#include <stdint.h>
#include <stddef.h>
#include <mm/slab.h>

struct vnode;

typedef struct {
	size_t (*read)(struct vnode *node, void *buf, uint64_t offset, uint64_t count);
	size_t (*write)(struct vnode *node, void *buf, uint64_t offset, uint64_t count);
	struct vnode *(*lookup)(struct vnode *node, const char *name);
} vops_t;

typedef struct vfs {
	char name[32];
	struct vfs *next;
	struct vnode *root;
	vops_t ops;
	slab_cache_t *cache;
	void *metadata; // Private file system info.
} vfs_t;

typedef struct vnode {
	char name[128];
	size_t size;
	vops_t *ops;
	vfs_t *vfs;
	struct vnode *parent;
	struct vnode *child;
	struct vnode *next;
	struct vnode *prev;
	void *metadata;
} vnode_t;

extern slab_cache_t *vfs_cache;

void vfs_init();
void vfs_register(vfs_t *vfs);

vnode_t *vfs_find(const char *path);

size_t vfs_read(vnode_t *node, void *buf, uint64_t offset, uint64_t count);
size_t vfs_write(vnode_t *node, void *buf, uint64_t offset, uint64_t count);
vnode_t *vfs_lookup(vnode_t *node, const char *name);
