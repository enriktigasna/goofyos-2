#include <goofy-os/hmap.h>
#include <goofy-os/list.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/spinlock.h>
#include <goofy-os/uapi/errno.h>
#include <goofy-os/vfs.h>
#include <string.h>

struct hashmap dentry_cache;
struct hashmap vnode_cache;

extern int tmpfs_mount(struct dentry *dentry, struct vfs *vfs);
void dentry_resolve(struct dentry *dentry, char *path) {
	struct dlist stack;
	memset(&stack, 0, sizeof(struct dlist));

	for (struct dentry *curr = dentry; curr; curr = curr->parent) {
		dlist_front_push(&stack, curr->name);
	}

	int len = 0;
	for (struct dnode *curr = stack.head; curr; curr = curr->next) {
		path += len;
		path[0] = '/';
		char *name = ((struct dentry *)curr->value)->name;
		strcpy(path + 1, name);
		len = strlen(name);
	}
}

struct dlist *vfs_parse_path(char *path) {
	struct dlist *ret = kzalloc(sizeof(struct dlist));
	char buf[VFS_PATH_MAX];
	if (path[0] == '/') {
		dlist_back_push(ret, strdup("/"));
		path++;
	}

	int curr = 0;
	for (int i = 0; path[i]; i++) {
		buf[curr] = path[i];
		if (path[i] == '/') {
			buf[curr] = '\0';
			dlist_back_push(ret, strdup(buf));
			curr = 0;
			continue;
		}
		curr++;
	}
	return ret;
}

int vfs_find_vnode(char *path, struct vnode **res) {}

int vfs_find_parent_vnode(char *path, struct vnode **res) {
	struct dlist *files = vfs_parse_path(path);
	kfree(dlist_back_pop(files)->value);
	// Traverse dcache until can't
	printk("vfs_find_parent_vnode(%s)\n", path);
	return -ENOSYS;
	// rootfs->children_cache
	// If / start at rootfs (and pop first), else start at dfd
	// dfd not implemented yet
	if (path == '/') {
	}
	// while files not empty
	// if dirent:
	// 	dirent = find_child_of_dirent_with_name
}

int vfs_mkdir(char *path) {
	struct vnode *parent;
	int err = vfs_find_parent_vnode(path, &parent);
	if (err)
		return err;
	return -1;
}

void vfs_cache_dentry(struct dentry *ent) {
	size_t size = sizeof(struct dentry *) + strlen(ent->name) + 1;
	struct dcache_key *key = kmalloc(size);
	key->parent = ent->parent;
	strcpy((char *)&key->name, ent->name);

	hmap_add(&dentry_cache, key, size, ent);
}

void vfs_cache_vnode(struct vnode *vnode) {
	struct vnode_key *key = kmalloc(sizeof(struct vnode_key));
	key->vfs = vnode->curr_vfs;
	key->number = vnode->number;

	hmap_add(&vnode_cache, key, sizeof(struct vnode_key), vnode);
}

long vfs_upcount;
// Use very sparingly, mostly when mounting to prevent same-vfs-id race
// condition
struct spinlock vfs_biglock;

void vfs_init() {
	hmap_init(&dentry_cache);
	hmap_init(&vnode_cache);

	struct vnode *node = kzalloc(sizeof(struct vnode));
	struct vfs *vfs = kzalloc(sizeof(struct vfs));
	node->mode = S_IFDIR | 0777;
	node->curr_vfs = vfs;

	struct dentry *root_dentry = kzalloc(sizeof(struct dentry));
	root_dentry->name = strdup("/");
	root_dentry->vnode = node;
	root_dentry->refcount = 1;

	vfs->root_dentry = root_dentry;
	vfs->id = vfs_upcount++;
	tmpfs_mount(root_dentry, vfs);

	vfs_cache_dentry(root_dentry);
	vfs_cache_vnode(node);
}