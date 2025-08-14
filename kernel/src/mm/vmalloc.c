#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/vmalloc.h>

struct vmalloc_struct *root;

// I use linked list for simple implementation, its good enough
// Eventually this might be a performance bottleneck,
// and I will switch to rb-tree
// Linux used this kind of approach until 2.6

void *vmalloc_map_range(size_t size) {
	if (size == 0 || size % PAGE_SIZE != 0) {
		return NULL;
	}

	struct vmalloc_struct *curr;
	for (curr = root; curr; curr = curr->next) {
		if (curr->flags & VM_INUSE) {
			continue;
		}

		if (curr->size == size) {
			// Just set set flag
			curr->flags |= VM_INUSE;
			return curr->addr;
		}

		if (curr->size > size) {
			// Create a new splitoff, at the back
			struct vmalloc_struct *new;
			new = kzalloc(sizeof(struct vmalloc_struct));
			new->addr = curr->addr + curr->size - size;
			new->next = curr->next;
			new->size = size;
			new->flags |= VM_INUSE;

			curr->next = new;
			curr->size -= size;
			return new->addr;
		}
	}

	return NULL;
}

void vmalloc_unmap_range(void *addr) {
	struct vmalloc_struct *curr;
	for (curr = root; curr; curr = curr->next) {
		if (curr->addr == addr) {
			curr->flags &= ~VM_INUSE;
		}
	}
}

void vmalloc_init() {
	root = kzalloc(sizeof(struct vmalloc_struct));
	root->addr = (void *)VMALLOC_START;
	root->size = VMALLOC_END - VMALLOC_START;

	void *vaddr;
}