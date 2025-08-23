#include <goofy-os/list.h>
#include <goofy-os/slab.h>
#include <goofy-os/uapi/errno.h>
#include <goofy-os/vfs.h>

struct tmpfs_inode {
	char *name;
	struct dlist *children;
	char *data;
	long length;
	short mode;
};

struct vnode_operations tmpfs_operations = {};

int tmpfs_mount(struct vnode *node, struct vfs *vfs) {
	if (node->type != V_DIRECTORY)
		return -ENOTDIR;

	struct tmpfs_inode *root_inode = kzalloc(sizeof(struct tmpfs_inode));
	root_inode->mode |= 1 << V_DIRECTORY;
	root_inode->children = kzalloc(sizeof(struct dlist));

	vfs->vfs_root = node;
	node->ops = &tmpfs_operations;
	node->curr_vfs = vfs;
	node->private_data = root_inode;
	node->refcount++; // Mountpoint is a reference

	return 0;
}