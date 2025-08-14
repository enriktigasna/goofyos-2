#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/spinlock.h>
#include <goofy-os/vmalloc.h>
#include <string.h>

struct vmalloc_struct *root;
struct spinlock vmalloc_lock;

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

void vmalloc_unmap_range(void *range) {
	struct vmalloc_struct *curr;
	for (curr = root; curr; curr = curr->next) {
		if (curr->addr == range) {
			curr->flags &= ~VM_INUSE;
		}
	}
}

// Unmap and free pages backed by virt
void _vrelease_pages(size_t count, void *virt) {
	for (int i = 0; i < count; i++) {
		void *curr_virt = virt + i * PAGE_SIZE;
		uint64_t curr_phys = virt_to_phys(curr_virt);

		void *hhdm_virt = (void *)curr_phys + hhdm_offset;
		pgfree(hhdm_virt);
		unmap_page(kernel_virtual_pt, curr_virt);
	}
}

void _vfree(void *range) {
	struct vmalloc_struct *curr;
	for (curr = root; curr; curr = curr->next) {
		if (curr->addr == range) {
			curr->flags &= ~VM_INUSE;
			printk("Releasing addr %p\n", curr->addr);
			printk("Releasing size %x\n", curr->size);
			_vrelease_pages(curr->size / PAGE_SIZE, curr->addr);
			return;
		}
	}
}

void vfree(void *range) {
	acquire(&vmalloc_lock);
	_vfree(range);
	release(&vmalloc_lock);
}

void *_vmalloc(size_t size, uint64_t flags) {
	size_t aligned_size = PAGE_COUNT(size) * PAGE_SIZE;
	void *addr = vmalloc_map_range(aligned_size);

	for (int i = 0; i < aligned_size / PAGE_SIZE; i++) {
		uint64_t phys = (uint64_t)pgalloc() - hhdm_offset;
		map_page(kernel_virtual_pt, phys, addr + PAGE_SIZE * i, flags);
	}

	return addr;
}

void *vmalloc_flags(size_t size, uint64_t flags) {
	acquire(&vmalloc_lock);
	void *virt = _vmalloc(size, flags);
	release(&vmalloc_lock);
	return virt;
}

void *vmalloc(size_t size) {
	acquire(&vmalloc_lock);
	void *range = _vmalloc(size, PG_WRITE | PG_NX);
	release(&vmalloc_lock);
	return range;
}

void *vzalloc(size_t size) {
	void *range = vmalloc(size);
	memset(range, 0, size);
	return range;
}

void vmalloc_init() {
	root = kzalloc(sizeof(struct vmalloc_struct));
	root->addr = (void *)VMALLOC_START;
	root->size = VMALLOC_END - VMALLOC_START;

	void *vaddr;
}