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
	if (curr > 0) {
		buf[curr] = '\0';
		dlist_back_push(ret, strdup(buf));
	}
	return ret;
}

int vfs_find_vnode(struct vnode *vnode, long num, struct vnode **res) {
	if (!vnode || !res || !vnode->ops || !vnode->curr_vfs)
		return EINVAL;

	struct vnode_key vkey = {.number = num, .vfs = vnode->curr_vfs};
	struct vnode *ret =
	    hmap_lookup(&vnode_cache, &vkey, sizeof(struct vnode_key));
	if (ret)
		goto SUCCESS;

	if (!vnode->ops->create_node)
		return -ENOSYS;

	int err = vnode->ops->create_node(vnode, num, &ret);
	if (err)
		return err;

SUCCESS:
	*res = ret;
	return 0;
}

int vfs_find_child(struct dentry *dent, char *name, struct dentry **res) {
	if (!dent || !res)
		return -EINVAL;

	struct dentry *ret;
	size_t dkey_size = sizeof(struct dentry *) + strlen(name) + 1;
	struct dcache_key *dkey = kmalloc(dkey_size);
	dkey->parent = dent;
	strcpy(dkey->name, name);

	ret = hmap_lookup(&dentry_cache, dkey, dkey_size);
	kfree(dkey);
	if (ret)
		goto SUCCESS;

	ret = kzalloc(sizeof(struct dentry));
	long num;
	struct vnode_operations *ops = dent->vnode->ops;
	int err = ops->lookup(dent->vnode, name, &num);
	if (err)
		return err;

	err = vfs_find_vnode(dent->vnode, num, &ret->vnode);
	if (err)
		return err;

SUCCESS:
	*res = ret;
	return 0;
}

int vfs_find_dentry(char *path, struct dentry **res, struct dentry *rel,
		    bool parent) {
	struct dlist *files = vfs_parse_path(path);
	printk("parsed %d from %s\n", files->count, path);

	if (!files->count || (files->count == 1 && parent)) {
		return -EINVAL;
	}

	printk("files count %d\n", files->count);

	if (parent) {
		kfree(files->tail->value);
		kfree(dlist_back_pop(files));
	}
	// Traverse dcache until can't
	printk("vfs_find_parent_vnode(%s)\n", path);

	if (path[0] == '/') {
		kfree(files->head->value);
		kfree(dlist_front_pop(files));
		rel = global_root;
	}

	if (path[0] == '.') {
		kfree(files->head->value);
		kfree(dlist_front_pop(files));
	}

	if (rel == NULL)
		rel = global_root;

	struct dentry *cur = rel;
	// TODO in future: .. representation
	// For efficient refcounting: have a lowest value, if we .. from rel,
	// then that will be the new rel (relative root we traverse from)
	// All new dentries need to be kept
	// in a list, and initialized to 0 refcount

	// After traversal is complete, walk from res to rel adding one refcount
	// to each by parent

	while (files->head) {
		struct dentry *res;
		int err = vfs_find_child(cur, files->head->value, &res);
		kfree(dlist_front_pop(files)->value);
		if (err) {
			dlist_kfree_values(files);
			return err;
		}

		cur = res;
	}

	*res = cur;
	return 0;
}
// VA for dcache key
//(files->head->value)

// 1. Check dcache, if found pop and continue

// 2. fs lookup (if nonexistant -EEXIST)
// 3. node cache lookup ? create

int vfs_mkdir(char *path, struct dentry *rel, short flags) {
	struct dentry *parent;
	int err = vfs_find_dentry(path, &parent, rel, true);
	if (err)
		return err;

	// We want it to error, this is how we check if it doesnt exist
	struct dlist *files = vfs_parse_path(path);
	char *child = strdup(files->tail->value);
	dlist_kfree_values(files);

	struct dentry *dummy;
	if (vfs_find_child(parent, child, &dummy)) {
		kfree(child);
		return -EEXIST;
	}

	return parent->vnode->ops->mkdir(parent->vnode, child, S_IFDIR | flags);
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