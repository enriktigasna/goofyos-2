#include <goofy-os/list.h>
#include <stdbool.h>

#define VFS_PATH_MAX 256

#define MODE_DIRECTORY 01000

extern struct dlist mountpoints;

struct vnode_operations {
	int (*open)(struct vnode *node, char *buf, int n);
	int (*read)(struct vnode *node, char *buf, int n);
	int (*write)(struct vnode *node, char *buf, int n);
	int (*ioctl)(struct vnode *node, int request, void *arg);
	int (*getdirents)(struct vnode *node, struct dlist *ents);
	int (*lookup)(struct vnode *node, char *name, struct vnode *res);
	int (*mkdir)(struct vnode *node, char *name);
	int (*create)(struct vnode *node, char *name);
	int (*delete)(struct vnode *node, char *buf, int n);
	int (*rename)(struct vnode *node, char *buf, int n);
	int (*free)(struct vnode *node);
	struct vfs *fs;
};

enum vnode_type {
	V_FILE,
	V_DIRECTORY,
	V_SYMLINK,
};

struct vnode {
	struct vnode_operations *ops;
	struct vnode *parent;
	struct vfs *curr_vfs;
	void *private_data;
	struct dlist *fds;
	int refcount;
	enum vnode_type type;
	bool inuse;
};

struct mountpoint {
	char *name;
	struct vfs *fs;
	struct mountpoint *next;
};

struct vfs {
	struct vnode *vfs_root;
	void *private_data;
};

struct file {
	struct vnode *node;
	int flags;
};

void vfs_init();