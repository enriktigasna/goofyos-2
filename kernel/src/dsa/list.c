#include <goofy-os/list.h>
#include <goofy-os/slab.h>
#include <stddef.h>
#include <stdint.h>

void dlist_front_push(struct dlist *stack, void *value) {
	struct dnode *new = kzalloc(sizeof(struct dnode));
	new->value = value;
	new->next = stack->head;
	if (stack->head)
		stack->head->prev = new;
	stack->head = new;
	if (!stack->tail)
		stack->tail = stack->head;
	stack->count++;
}

struct dnode *dlist_front_pop(struct dlist *stack) {
	struct dnode *ret = stack->head;
	if (ret->next)
		ret->next->prev = NULL;
	ret->next = NULL;
	stack->head = stack->head->next;
	if (!stack->head)
		stack->tail = NULL;
	stack->count--;
}

void dlist_back_push(struct dlist *queue, void *value) {
	struct dnode *new = kzalloc(sizeof(struct dnode));
	new->value = value;
	new->prev = queue->tail;
	if (queue->tail)
		queue->tail->next = new;
	queue->tail = new;
	if (!queue->head)
		queue->head = queue->tail;
	queue->count++;
}

struct dnode *dlist_back_pop(struct dlist *queue) {
	struct dnode *ret = queue->tail;
	if (ret->prev)
		ret->prev->next = NULL;
	ret->prev = NULL;
	queue->tail = queue->tail->prev;
	if (!queue->tail)
		queue->head = NULL;
	queue->count--;
}

void dlist_kfree_values(struct dlist *dlist) {
	for (struct dnode *curr = dlist->head; curr; curr->next) {
		kfree(curr->value);
	}
	kfree(dlist);
}

void dlist_destroy_values(struct dlist *dlist, void (*destructor)(void *)) {
	for (struct dnode *curr = dlist->head; curr; curr->next) {
		destructor(curr->value);
		kfree(curr->value);
	}
	kfree(dlist);
}

void dlist_remove_item(struct dlist *dlist, struct dnode *item) {
	if (item == dlist->head)
		return dlist_front_pop(dlist);
	if (item == dlist->tail)
		return dlist_back_pop(dlist);

	item->next->prev = item->prev;
	item->prev->next = item->next;
	dlist->count--;
}