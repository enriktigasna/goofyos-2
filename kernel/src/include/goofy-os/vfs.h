#include <goofy-os/list.h>
#include <goofy-os/spinlock.h>
#include <goofy-os/uapi/stat.h>
#include <stdbool.h>

#define VFS_PATH_MAX 256

struct vnode {
	struct spinlock lock;
	struct vnode_operations *ops;
	struct vfs *curr_vfs;
	void *private_data;
	struct dlist *fds;
	int refcount;
	short mode;
	bool inuse;
	long number;
};

struct vfs {
	char *name;
	long id;
	struct dentry *root_dentry;
	void *private_data;
	struct spinlock lock;
};

struct dentry {
	char *name;
	struct dentry *parent;
	struct vnode *vnode;
	long refcount;
	bool negative;
};

struct dcache_key {
	struct dentry *parent;
	char name[];
};

struct vnode_key {
	struct vfs *vfs;
	long number;
};

struct mountpoint {
	struct dentry *mount_dentry;
	struct vfs *fs;
	struct mountpoint *next;
};

void vfs_init();
void dentry_resolve(struct dentry *dentry, char *path);

struct vnode_operations {
	int (*open)(struct vnode *node, short flags);
	int (*read)(struct vnode *node, char *buf, long n, long off);
	int (*write)(struct vnode *node, char *buf, long n, long off);
	int (*ioctl)(struct vnode *node, int request, void *arg);
	int (*getdirents)(struct vnode *node, struct dlist *list);
	int (*lookup)(struct vnode *node, char *name, long *num);
	int (*mkdir)(struct vnode *node, char *name, short flags);
	int (*create)(struct vnode *node, char *name, short flags);
	int (*chmod)(struct vnode *node, short flags);
	int (*remove)(struct vnode *node);
	int (*rmdir)(struct vnode *node);
	int (*move)(struct vnode *node, struct dentry dentry);
	int (*create_node)(struct vnode *node, long num, struct vnode **res);
	int (*destroy_node)(struct vnode *node);
};

struct dirent {
	char *name;
	short mode;
};

struct file {
	struct dentry *entry;
	long pos;
	short flags;
};

void unpack_initrd();

/**
 * These helpers will trust what they are given, permissions, O_CREAT flag, etc.
 * are to be handled in a higher level abstraction
 */
int vfs_mkdir(char *path, struct dentry *rel, short flags);
int vfs_create(char *path, struct dentry *rel, short flags);
int vfs_open(char *path, struct dentry *rel, short flags, struct file *fd);