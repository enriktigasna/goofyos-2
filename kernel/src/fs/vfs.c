#include <goofy-os/list.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/vfs.h>
#include <string.h>

/**
 * GoofyOS VFS
 *
 * The VFS does not have mountpoints, it is instead a tree of dirent caches. All
 * mountpoints are guaranteed to be found by just walking this tree. This is so
 * that you won't try to create a node through doing a lookup in wrong fs, and
 * messing up state.
 *
 * If a dentry cache doesn't have an entry you are looking for,
 * you go into it's backing vnode, and do lookup.
 *
 * When removing, it creates a negative dentry, so that when all
 * references close, it will call vnode->remove
 *
 */

struct dentry *rootfs;

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

void vfs_init() {
	// Init tmpfs to root and add it to mountpoints
	struct vnode *node = kzalloc(sizeof(struct vnode));
	struct vfs *vfs = kzalloc(sizeof(struct vfs));
	node->mode = S_IFDIR | 0777;
	node->curr_vfs = vfs;

	struct dentry *root_dentry = kzalloc(sizeof(struct dentry));
	root_dentry->name = strdup("/");
	root_dentry->vnode = node;
	root_dentry->refcount = 1;
	rootfs = root_dentry;

	vfs->root_dentry = root_dentry;
	tmpfs_mount(root_dentry, vfs);
}