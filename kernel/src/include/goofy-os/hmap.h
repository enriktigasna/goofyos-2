#include <stddef.h>
#include <stdint.h>

struct hashmap {
	size_t size;
	size_t count;
	struct dlist *buf;
	uint64_t (*hash_func)(void *data, size_t size);
};

void hmap_init(struct hashmap *hashmap);

void hmap_add(struct hashmap *hashmap, void *key, size_t key_size, void *value);
void *hmap_lookup(struct hashmap *hashmap, void *key, size_t key_size);
void hmap_remove(struct hashmap *hashmap, void *key, size_t key_size);