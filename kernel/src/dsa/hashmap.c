#include <goofy-os/list.h>
#include <goofy-os/slab.h>
#include <stddef.h>
#include <stdint.h>

#define FNV_PRIME 0x00000100000001b3
#define FNV_OFFSET 0xcbf29ce484222325

uint64_t fnva_hash(char *data, size_t size) {
	uint64_t hash = FNV_OFFSET;
	for (int i = 0; i < size; i++) {
		hash ^= data[i];
		hash *= FNV_PRIME;
	}
	return hash;
}

struct hashmap {
	size_t size;
	size_t count;
	struct dlist *buf;
};

struct hashmap_entry {
	struct hashmap_entry *next;
	uint64_t hash;
	void *key;
	void *value;
};

struct hashmap *hmap_init() {
	// Init with one page from there just double
	struct hashmap *ret = kmalloc(sizeof(struct hashmap));
	ret->buf = vmalloc(0x1000);
	ret->size = 0x1000 / sizeof(struct dlist);
	ret->count = 0;
}

void hmap_resize(struct hashmap *hashmap, size_t new_size) {}

void hmap_add(struct hashmap *hashmap, void *key, void *value) { if (size->) }