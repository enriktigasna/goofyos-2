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
	int (*open)(struct vnode *node, char *buf, long n);
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
	short flags;
};

void unpack_initrd();

int vfs_mkdir(char *path, struct dentry *rel);