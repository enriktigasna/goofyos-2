#include <goofy-os/list.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/vfs.h>
#include <string.h>

/**
 * GoofyOS VFS
 *
 * The VFS does not have mountpoints, it is instead a tree of dirent caches. All
 * mountpoints are guaranteed to be found by just walking this tree.
 *
 * If a dentry cache doesn't have an entry you are looking for,
 * you go into it's backing vnode, and do lookup.
 *
 * Getdirents doesn't return
 * a bunch of dirents, intead it fill the dirent cache children with new
 * dirents, also incrementing all refcounts by one.
 *
 * After a getdirent call you can extract all the data from the children, then
 * put all new dirents.
 *
 *
 */

struct mountpoint mount_list;

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

void vfs_init() {
	// Init tmpfs to root and add it to mountpoints
	struct vnode *node = kzalloc(sizeof(struct vnode));
	struct vfs *vfs = kzalloc(sizeof(struct vfs));
	node->type = V_DIRECTORY;
	node->curr_vfs = vfs;

	struct dentry *root_dentry = kzalloc(sizeof(struct dentry));
	root_dentry->name = strdup("/");
	root_dentry->vnode = node;
	root_dentry->refcount = 1;
	mount_list.mount_dentry = root_dentry;
	mount_list.fs = vfs;

	vfs->root_dentry = root_dentry;
	tmpfs_mount(root_dentry, vfs);
}