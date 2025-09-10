#include <goofy-os/hmap.h>
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

int tmpfs_mkdir(struct vnode *node, char *name, short flags);
int tmpfs_lookup(struct vnode *node, char *name, long *num);
int tmpfs_create_node(struct vnode *node, long num, struct vnode **res);
int tmpfs_create(struct vnode *node, char *name, short flags);
int tmpfs_read(struct vnode *node, char *buf, long n, long off);
int tmpfs_write(struct vnode *node, char *buf, long n, long off);

struct vnode_operations tmpfs_operations = {
    .mkdir = tmpfs_mkdir,
    .create = tmpfs_create,
    .lookup = tmpfs_lookup,
    .create_node = tmpfs_create_node,
    .read = tmpfs_read,
    .write = tmpfs_write,
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
	struct hashmap *tnode_cache;
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
	child_tnode->children = kzalloc(sizeof(struct dlist));
	acquire(&node->curr_vfs->lock);
	child_tnode->number = tblock->current_inode++;
	release(&node->curr_vfs->lock);
	dlist_back_push(tnode->children, child_tnode);

	hmap_add(tblock->tnode_cache, &child_tnode->number, sizeof(long),
		 child_tnode);

	release(&node->lock);
	return 0;
}

int tmpfs_create(struct vnode *node, char *name, short flags) {
	acquire(&node->lock);

	struct tmpfs_inode *tnode = node->private_data;
	struct tmpfs_superblock *tblock = node->curr_vfs->private_data;

	if (tmpfs_has_file(tnode, name))
		return -EEXIST;

	struct tmpfs_inode *child_tnode = kzalloc(sizeof(struct tmpfs_inode));
	child_tnode->name = strdup(name);
	child_tnode->mode = (flags & ~S_IFMT) | S_IFREG;
	child_tnode->children = kzalloc(sizeof(struct dlist));
	acquire(&node->curr_vfs->lock);
	child_tnode->number = tblock->current_inode++;
	release(&node->curr_vfs->lock);
	dlist_back_push(tnode->children, child_tnode);

	hmap_add(tblock->tnode_cache, &child_tnode->number, sizeof(long),
		 child_tnode);

	release(&node->lock);
	return 0;
}

int tmpfs_read(struct vnode *node, char *buf, long n, long off) {
	if (!S_ISREG(node->mode)) {
		return -EINVAL;
	}

	struct tmpfs_inode *tnode = node->private_data;
	if (off >= tnode->length) {
		return 0;
	}

	long available = tnode->length - off;
	long to_read = MIN(n, available);

	printk("Trying to read %d from %d long file\n", to_read, tnode->length);
	memcpy(buf, tnode->data, to_read);
	return to_read;
}

int tmpfs_write(struct vnode *node, char *buf, long n, long off) {
	if (!S_ISREG(node->mode)) {
		return -EINVAL;
	}

	struct tmpfs_inode *tnode = node->private_data;
	long new_length = MAX(tnode->length, off + n);

	if (new_length > tnode->effective_length) {
		size_t alloc_size = (new_length + 0xfff) & ~0xfff;
		char *new = vmalloc(alloc_size);
		memcpy(new, tnode->data, tnode->length);
		vfree(tnode->data);
		tnode->data = new;
		tnode->effective_length = alloc_size;
	}

	memcpy(&tnode->data[off], buf, n);

	tnode->length = new_length;
	return n;
}

int tmpfs_lookup(struct vnode *node, char *name, long *num) {
	acquire(&node->lock);
	struct tmpfs_inode *tnode = node->private_data;

	struct dlist *children = tnode->children;
	for (struct dnode *curr = children->head; curr; curr = curr->next) {
		struct tmpfs_inode *curr_tnode = curr->value;
		if (!strcmp(curr_tnode->name, name)) {
			release(&node->lock);
			*num = curr_tnode->number;
			return 0;
		}
	}

	release(&node->lock);
	return -ENOENT;
}

int tmpfs_create_node(struct vnode *node, long num, struct vnode **res) {
	struct tmpfs_superblock *tblock = node->curr_vfs->private_data;
	struct tmpfs_inode *tnode =
	    hmap_lookup(tblock->tnode_cache, &num, sizeof(long));
	if (!tnode) {
		return -ENOENT;
	}

	struct vnode *new_node = kzalloc(sizeof(struct vnode));
	new_node->ops = &tmpfs_operations;
	new_node->curr_vfs = node->curr_vfs;
	new_node->number = num;
	new_node->mode = tnode->mode;
	new_node->private_data = tnode;
	new_node->refcount = 1;

	*res = new_node;

	return 0;
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

	((struct tmpfs_superblock *)vfs->private_data)->tnode_cache =
	    kzalloc(sizeof(struct hashmap));
	hmap_init(((struct tmpfs_superblock *)vfs->private_data)->tnode_cache);
	hmap_add(((struct tmpfs_superblock *)vfs->private_data)->tnode_cache,
		 &root_inode->number, sizeof(long), root_inode);

	char path[VFS_PATH_MAX + 1];
	dentry_resolve(dentry, path);
	printk("Mounted tmpfs into folder %s\n", path);
}
