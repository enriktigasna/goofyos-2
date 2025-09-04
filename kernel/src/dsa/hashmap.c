#include <goofy-os/hmap.h>
#include <goofy-os/list.h>
#include <goofy-os/slab.h>
#include <goofy-os/vmalloc.h>
#include <stdint.h>
#include <string.h>

#define FNV_PRIME 0x00000100000001b3
#define FNV_OFFSET 0xcbf29ce484222325

uint64_t fnva_hash(void *data, size_t size) {
	char *cdata = (char *)data;

	uint64_t hash = FNV_OFFSET;
	for (int i = 0; i < size; i++) {
		hash ^= cdata[i];
		hash *= FNV_PRIME;
	}
	return hash;
}

/**
 * These hashmap entries are agnostic to where they are
 * Their references will be moved within the hashmap array,
 * but a reference to the hashmap_entry is valid while the
 * hashmap is valid.
 */
struct hashmap_entry {
	uint64_t hash;
	void *key;
	size_t key_size;
	void *value;
};

struct hashmap *hmap_init() {
	// Init with one page from there just double
	struct hashmap *ret = kmalloc(sizeof(struct hashmap));
	ret->buf = vzalloc(0x1000);
	ret->size = 0x1000 / sizeof(struct dlist);
	ret->count = 0;
	ret->hash_func = fnva_hash;
	return ret;
}

void hmap_insert_arr(struct dlist *arr, size_t size, uint64_t hash) {}

void hmap_resize(struct hashmap *hashmap) {
	struct dlist *new = vzalloc(hashmap->size * 2 * sizeof(struct dlist));
	for (int i = 0; i < hashmap->size; i++) {
		struct dlist *curr = &hashmap->buf[i];
		while (curr->count) {
			struct dnode *head = dlist_back_pop(curr);
			struct hashmap_entry *ent = head->value;
			dlist_back_push(&new[ent->hash % (hashmap->size * 2)],
					ent);
		}
	}
	hashmap->size *= 2;
	vfree(hashmap->buf);
	hashmap->buf = new;
}

void hmap_add(struct hashmap *hashmap, void *key, size_t key_size,
	      void *value) {
	if (hashmap->count >= hashmap->size) {
		hmap_resize(hashmap);
	}

	uint64_t hash = hashmap->hash_func(key, key_size);
	struct dlist *cur = &hashmap->buf[hash % hashmap->size];
	struct hashmap_entry *new_ent = kzalloc(sizeof(struct hashmap_entry));
	new_ent->value = value;
	new_ent->hash = hash;
	new_ent->key = key;
	new_ent->key_size = key_size;

	dlist_back_push(cur, new_ent);
}

void *hmap_lookup(struct hashmap *hashmap, void *key, size_t key_size) {
	uint64_t hash = hashmap->hash_func(key, key_size);
	struct dlist *ent = &hashmap->buf[hash % hashmap->size];
	struct dnode *curr = ent->head;
	while (curr) {
		struct hashmap_entry *cur_ent = curr->value;
		if (cur_ent->key_size != key_size) {
			curr = curr->next;
			continue;
		}
		if (!memcmp(key, cur_ent->key, key_size)) {
			return cur_ent->value;
		}

		curr = curr->next;
		continue;
	}
	return NULL;
}