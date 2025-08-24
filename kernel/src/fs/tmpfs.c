#include <goofy-os/list.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/uapi/errno.h>
#include <goofy-os/vfs.h>
#include <string.h>

struct tmpfs_inode {
	char *name;
	struct dlist *children;
	char *data;
	long length;
	short mode;
};

struct vnode_operations tmpfs_operations = {};

bool tmpfs_has_file(struct tmpfs_inode *node) {
	struct dlist *children = node->children;
	for (struct dnode *curr = children->head; curr; curr = curr->next) {
		if (((struct tmpfs_inode *)curr->value)->name) {
		}
	}
}

void tmpfs_create(struct vnode *node, char *name, short flags) {
	struct tmpfs_inode *tmpfs_node = node->private_data;
	tmpfs_node->children;
}

void tmpfs_mount(struct dentry *dentry, struct vfs *vfs) {
	struct vnode *root_vnode = kzalloc(sizeof(struct vnode));
	struct tmpfs_inode *root_inode = kzalloc(sizeof(struct tmpfs_inode));
	dentry->vnode = root_vnode;
	root_vnode->private_data = root_inode;
	root_vnode->type = V_DIRECTORY;
	root_vnode->ops = &tmpfs_operations;
	root_vnode->refcount = 1;

	root_inode->children = kzalloc(sizeof(struct dlist));
	root_inode->name = dentry->name;

	char path[VFS_PATH_MAX + 1];
	dentry_resolve(dentry, path);
	printk("Mounted tmpfs into folder %s\n", path);
}