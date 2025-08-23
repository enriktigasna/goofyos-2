#include <goofy-os/list.h>
#include <goofy-os/slab.h>
#include <goofy-os/vfs.h>
#include <string.h>

struct mountpoint mount_list;

extern int tmpfs_mount(struct vnode *node, struct vfs *vfs);

void vfs_init() {
	// Init tmpfs to root and add it to mountpoints
	struct vnode *node = kzalloc(sizeof(struct vnode));
	node->type = V_DIRECTORY;
	struct vfs *vfs = kzalloc(sizeof(struct vfs));
	tmpfs_mount(node, vfs);

	// First mount is always /
	mount_list.name = strdup("/");
	mount_list.fs = vfs;
}