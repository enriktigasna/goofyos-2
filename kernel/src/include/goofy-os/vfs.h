#include <goofy-os/list.h>
#include <goofy-os/uapi/stat.h>
#include <stdbool.h>

#define VFS_PATH_MAX 256

extern struct dentry *rootfs;

struct vnode {
	struct vnode_operations *ops;
	struct vfs *curr_vfs;
	void *private_data;
	struct dlist *fds;
	int refcount;
	short mode;
	bool inuse;
};

struct vfs {
	char *name;
	struct dentry *root_dentry;
	void *private_data;
};

struct dentry {
	char *name;
	struct dentry *parent;
	struct dlist *children_cache;
	struct vnode *vnode;
	int refcount;
	bool negative;
};

struct mountpoint {
	struct dentry *mount_dentry;
	struct vfs *fs;
	struct mountpoint *next;
};

void vfs_init();
void dentry_resolve(struct dentry *dentry, char *path);

/**
 * struct vnode_operations - Filesystem defined operations on vnode
 *
 * @open: Optional hook to be called on every open
 * @read: Read from file
 * @write: Writes to file
 * @ioctl: Ioctls file
 * @getdirents: Get all entries below vnode, fills list with dirent. Returns how
 * many it got.
 * @lookup: Lookup specific child of a vnode. This will create a new vnode,
 * check dentry first to avoid duplicates.
 * @mkdir: Create directory under vnode
 * @create: Create file under vnode. Does not create backing dentry, to get a
 * reference to the new file you have to lookup and get one.
 * @chmod: Change file flags on vnode
 * @remove: Remove vnode, if file
 * @rmdir: Remove vnode, if directory
 * @move: Move vnode to new dentry.
 * 	Dentry guaranteed to be in same filesystem, otherwise OS will create new
 * 	files and do copy operations.
 *
 * 	Implementations resolve dentry in reference to
 * 	node->curr_vfs->root_dentry, and move this way. This is a common pattern
 * 	so there will be helpers for it.
 * @free: Optional hook to be called when freeing vnode struct, for example to
 * 	clean vnode->private_data.
 *
 */
struct vnode_operations {
	int (*open)(struct vnode *node, char *buf, long n);
	int (*read)(struct vnode *node, char *buf, long n, long off);
	int (*write)(struct vnode *node, char *buf, long n, long off);
	int (*ioctl)(struct vnode *node, int request, void *arg);
	int (*getdirents)(struct vnode *node, struct dlist *list);
	int (*lookup)(struct vnode *node, char *name, struct vnode **res);
	int (*mkdir)(struct vnode *node, char *name, short flags);
	int (*create)(struct vnode *node, char *name, short flags);
	int (*chmod)(struct vnode *node, short flags);
	int (*remove)(struct vnode *node);
	int (*rmdir)(struct vnode *node);
	int (*move)(struct vnode *node, struct dentry dentry);
	int (*free)(struct vnode *node);
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

int vfs_mkdir(char *path);