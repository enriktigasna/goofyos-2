#include <goofy-os/list.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/uapi/errno.h> #include <goofy-os/uapi/stat.h>
#include <goofy-os/vfs.h>
#include <goofy-os/vmalloc.h>
#include <string.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

int tmpfs_mkdir(struct vnode *node, char *name, short flags);
struct vnode_operations tmpfs_operations = {
    .mkdir = tmpfs_mkdir,
};

struct tmpfs_inode {
	char *name;
	struct dlist *children;
	char *data;
	long number;
	long length;
	long effective_length;
	short mode;
};

struct tmpfs_superblock {
	long current_inode;
};

bool tmpfs_has_file(struct tmpfs_inode *node, char *name) {
	struct dlist *children = node->children;
	for (struct dnode *curr = children->head; curr; curr = curr->next) {
		if (!strcmp(((struct tmpfs_inode *)curr->value)->name, name)) {
			return true;
		}
	}
	return false;
}

int tmpfs_mkdir(struct vnode *node, char *name, short flags) {
	acquire(&node->lock);

	struct tmpfs_inode *tnode = node->private_data;
	struct tmpfs_superblock *tblock = node->curr_vfs->private_data;

	if (tmpfs_has_file(tnode, name))
		return -EEXIST;

	struct tmpfs_inode *child_tnode = kzalloc(sizeof(struct tmpfs_inode));
	child_tnode->name = strdup(name);
	child_tnode->mode = (flags & ~S_IFMT) | S_IFDIR;
	acquire(&node->curr_vfs->lock);
	child_tnode->number = tblock->current_inode++;
	release(&node->curr_vfs->lock);
	dlist_back_push(tnode->children, child_tnode);

	release(&node->lock);
}

void tmpfs_mount(struct dentry *dentry, struct vfs *vfs) {
	struct vnode *root_vnode = kzalloc(sizeof(struct vnode));
	struct tmpfs_inode *root_inode = kzalloc(sizeof(struct tmpfs_inode));
	dentry->vnode = root_vnode;
	root_vnode->private_data = root_inode;
	root_vnode->mode = S_IFDIR | 0777;
	root_vnode->ops = &tmpfs_operations;
	root_vnode->refcount = 1;
	root_vnode->curr_vfs = vfs;

	root_inode->children = kzalloc(sizeof(struct dlist));
	root_inode->name = dentry->name;

	vfs->private_data = kzalloc(sizeof(struct tmpfs_superblock));
	root_inode->number =
	    ((struct tmpfs_superblock *)vfs->private_data)->current_inode++;

	char path[VFS_PATH_MAX + 1];
	dentry_resolve(dentry, path);
	printk("Mounted tmpfs into folder %s\n", path);
}