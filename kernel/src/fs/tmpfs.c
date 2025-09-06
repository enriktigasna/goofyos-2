#include <goofy-os/list.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/uapi/errno.h>
#include <goofy-os/uapi/stat.h>
#include <goofy-os/vfs.h>
#include <goofy-os/vmalloc.h>
#include <string.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

int tmpfs_create(struct vnode *node, char *name, short flags);
int tmpfs_mkdir(struct vnode *node, char *name, short flags);
int tmpfs_getdirents(struct vnode *node, struct dlist *list);
int tmpfs_write(struct vnode *node, char *buf, long n, long off);
int tmpfs_read(struct vnode *node, char *buf, long n, long off);
struct vnode_operations tmpfs_operations = {
    .create = tmpfs_create,
    .mkdir = tmpfs_mkdir,
    .getdirents = tmpfs_getdirents,
    .write = tmpfs_write,
    .read = tmpfs_read,
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

bool tmpfs_has_file(struct tmpfs_inode *node, char *name) {
	struct dlist *children = node->children;
	for (struct dnode *curr = children->head; curr; curr = curr->next) {
		if (!strcmp(((struct tmpfs_inode *)curr->value)->name, name)) {
			return true;
		}
	}
	return false;
}

struct vnode *tmpfs_new_vnode(struct tmpfs_inode *node, struct vfs *vfs) {
	struct vnode *new = kzalloc(sizeof(struct vnode));
	new->private_data = node;
	new->curr_vfs = vfs;
	new->refcount = 1;
	new->mode = node->mode;
	new->ops = &tmpfs_operations;
	return new;
}

int tmpfs_create(struct vnode *node, char *name, short flags) {
	if (!S_ISDIR(node->mode))
		return -ENOTDIR;

	struct tmpfs_inode *tmpfs_node = node->private_data;
	if (tmpfs_has_file(tmpfs_node, name))
		return -EEXIST;

	struct tmpfs_inode *new = kzalloc(sizeof(struct tmpfs_inode));
	new->name = strdup(name);
	new->mode = (flags & ~S_IFMT) | S_IFREG;
	dlist_back_push(tmpfs_node->children, new);

	return 0;
}

int tmpfs_mkdir(struct vnode *node, char *name, short flags) {
	if (!S_ISDIR(node->mode))
		return -ENOTDIR;

	struct tmpfs_inode *tmpfs_node = node->private_data;
	if (tmpfs_has_file(tmpfs_node, name))
		return -EEXIST;

	struct tmpfs_inode *new = kzalloc(sizeof(struct tmpfs_inode));
	new->name = strdup(name);
	new->mode = (flags & ~S_IFMT) | S_IFDIR;
	dlist_back_push(tmpfs_node->children, new);
}

int tmpfs_write(struct vnode *node, char *buf, long n, long off) {
	if (S_ISDIR(node->mode)) {
		return -EISDIR;
	}

	if (n == 0) {
		return 0;
	}

	struct tmpfs_inode *tnode = node->private_data;
	long target_length = MAX(n + off, tnode->length);
	if (target_length > tnode->effective_length) {
		// Reallocate file buffer to fit inside size
		long new_effective_length =
		    (target_length + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
		char *new_buf = vzalloc(new_effective_length);
		memcpy(new_buf, tnode->data, tnode->length);
		if (tnode->data)
			vfree(tnode->data);
		tnode->data = new_buf;
		tnode->effective_length = new_effective_length;
	}
	tnode->length = target_length;
	memcpy(tnode->data + off, buf, n);
	return n;
}

int tmpfs_read(struct vnode *node, char *buf, long n, long off) {
	if (S_ISDIR(node->mode)) {
		return -EISDIR;
	}

	if (n == 0) {
		return 0;
	}

	struct tmpfs_inode *tnode = node->private_data;
	int amount_to_read = MIN(n, n - ((n + off) - tnode->length));
	memcpy(buf, tnode->data + off, amount_to_read);
	return amount_to_read;
}

int tmpfs_lookup(struct vnode *node, char *name, struct vnode **res) {
	if (!S_ISDIR(node->mode))
		return -ENOTDIR;

	struct tmpfs_inode *tmpfs_node = node->private_data;
	struct dlist *children = tmpfs_node->children;
	for (struct dnode *curr = children->head; curr; curr = curr->next) {
		if (!strcmp(((struct tmpfs_inode *)curr->value)->name, name)) {
			// ADD DENTRY ENTRY WITH VNODE AND SET A RES
			*res = tmpfs_new_vnode(curr->value, node->curr_vfs);
			return 0;
		}
	}

	return -ENOENT;
}

int tmpfs_getdirents(struct vnode *node, struct dlist *list) {
	if (!S_ISDIR(node->mode))
		return -ENOTDIR;

	struct tmpfs_inode *tnode = (struct tmpfs_inode *)node->private_data;
	struct dlist *children = tnode->children;
	int n = 0;
	for (struct dnode *curr = children->head; curr; curr = curr->next) {
		struct dirent *dirent = kzalloc(sizeof(struct dirent));
		struct tmpfs_inode *child_tnode =
		    (struct tmpfs_inode *)curr->value;

		char *name = strdup(child_tnode->name);
		dirent->name = name;
		dlist_back_push(list, dirent);
		n++;
	}

	return n;
}

void tmpfs_mount(struct dentry *dentry, struct vfs *vfs) {
	struct vnode *root_vnode = kzalloc(sizeof(struct vnode));
	struct tmpfs_inode *root_inode = kzalloc(sizeof(struct tmpfs_inode));
	dentry->vnode = root_vnode;
	root_vnode->private_data = root_inode;
	root_vnode->mode = S_IFDIR | 0777;
	root_vnode->ops = &tmpfs_operations;
	root_vnode->refcount = 1;

	root_inode->children = kzalloc(sizeof(struct dlist));
	root_inode->name = dentry->name;

	char path[VFS_PATH_MAX + 1];
	dentry_resolve(dentry, path);
	printk("Mounted tmpfs into folder %s\n", path);
}