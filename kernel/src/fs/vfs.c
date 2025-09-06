#include <goofy-os/hmap.h>
#include <goofy-os/list.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/spinlock.h>
#include <goofy-os/uapi/errno.h>
#include <goofy-os/vfs.h>
#include <string.h>

struct hashmap dentry_cache;
struct hashmap vnode_cache;

struct dentry *global_root;

extern int tmpfs_mount(struct dentry *dentry, struct vfs *vfs);
void dentry_resolve(struct dentry *dentry, char *path) {
	struct dlist stack;
	memset(&stack, 0, sizeof(struct dlist));

	for (struct dentry *curr = dentry; curr; curr = curr->parent) {
		dlist_front_push(&stack, curr->name);
	}

	int len = 0;
	for (struct dnode *curr = stack.head; curr; curr = curr->next) {
		path += len;
		path[0] = '/';
		char *name = ((struct dentry *)curr->value)->name;
		strcpy(path + 1, name);
		len = strlen(name);
	}
}

struct dlist *vfs_parse_path(char *path) {
	struct dlist *ret = kzalloc(sizeof(struct dlist));
	char buf[VFS_PATH_MAX];
	if (path[0] == '/') {
		dlist_back_push(ret, strdup("/"));
		path++;
	}

	int curr = 0;
	for (int i = 0; path[i]; i++) {
		buf[curr] = path[i];
		if (path[i] == '/') {
			buf[curr] = '\0';
			dlist_back_push(ret, strdup(buf));
			curr = 0;
			continue;
		}
		curr++;
	}
	return ret;
}

int vfs_find_vnode(char *path, struct vnode **res) {}

// TODO add boolean to find_vnode that is if it should only find parent
int vfs_find_parent_vnode(char *path, struct vnode **res, struct dentry *rel) {
	struct dlist *files = vfs_parse_path(path);
	kfree(dlist_back_pop(files)->value);
	// Traverse dcache until can't
	printk("vfs_find_parent_vnode(%s)\n", path);

	if (path[0] == '/') {
		free(dlist_front_pop(files));
		rel = global_root;
	}

	if (rel == NULL) {
		return -EINVAL;
	}

	struct dentry *cur = rel;
	// TODO: .. implementation, that also puts parent, and checks for root
	while (files->head) {
		size_t size =
		    sizeof(struct dentry *) + strlen(files->head->value) + 1;
		struct dcache_key *key = kmalloc(size);
		key->parent = cur;
		strcpy(&key->name, files->head->value);

		void *res;
		if (res = hmap_lookup(&dentry_cache, key, size)) {
			cur = res;
			kfree(key);
			continue;
		}
		kfree(key);

		// didn't find in dcache, continue
		struct vnode_operations *ops = cur->vnode->ops;
		if (!ops->lookup)
			return -ENOSYS;

		long num;
		int err = ops->lookup(cur->vnode, files->head->value, num);
		if (err)
			return err;

		// check vnode cache otherwise create
		struct vnode_key vkey = {.number = num,
					 .vfs = cur->vnode->curr_vfs};

		if (res = hmap_lookup(&vnode_cache, key, size)) {
			// create dcache with it
		}
	}
}
// VA for dcache key
//(files->head->value)

// 1. Check dcache, if found pop and continue

// 2. fs lookup (if nonexistant -EEXIST)
// 3. node cache lookup ? create

int vfs_mkdir(char *path, struct dentry *rel) {
	struct vnode *parent;
	int err = vfs_find_parent_vnode(path, &parent, rel);
	if (err)
		return err;
	return -1;
}

void vfs_cache_dentry(struct dentry *ent) {
	size_t size = sizeof(struct dentry *) + strlen(ent->name) + 1;
	struct dcache_key *key = kmalloc(size);
	key->parent = ent->parent;
	strcpy((char *)&key->name, ent->name);

	hmap_add(&dentry_cache, key, size, ent);
}

void vfs_cache_vnode(struct vnode *vnode) {
	struct vnode_key *key = kmalloc(sizeof(struct vnode_key));
	key->vfs = vnode->curr_vfs;
	key->number = vnode->number;

	hmap_add(&vnode_cache, key, sizeof(struct vnode_key), vnode);
}

long vfs_upcount;
// Use very sparingly, mostly when mounting to prevent same-vfs-id race
// condition
struct spinlock vfs_biglock;

void vfs_init() {
	hmap_init(&dentry_cache);
	hmap_init(&vnode_cache);

	struct vnode *node = kzalloc(sizeof(struct vnode));
	struct vfs *vfs = kzalloc(sizeof(struct vfs));
	node->mode = S_IFDIR | 0777;
	node->curr_vfs = vfs;

	struct dentry *root_dentry = kzalloc(sizeof(struct dentry));
	root_dentry->name = strdup("/");
	root_dentry->vnode = node;
	root_dentry->refcount = 1;

	vfs->root_dentry = root_dentry;
	vfs->id = vfs_upcount++;
	tmpfs_mount(root_dentry, vfs);

	vfs_cache_dentry(root_dentry);
	vfs_cache_vnode(node);

	global_root = root_dentry;
}