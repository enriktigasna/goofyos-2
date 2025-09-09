#pragma once
#include <stddef.h>
#include <stdint.h>

struct dnode {
	void *value;
	struct dnode *next;
	struct dnode *prev;
};

/**
 * Double ended queue
 * Can be used as a queue, stack, list
 *
 * Locks not provided. To be locked by consumer.
 */
struct dlist {
	size_t count;
	struct dnode *head;
	struct dnode *tail;
};

void dlist_front_push(struct dlist *stack, void *value);
void *dlist_front_pop(struct dlist *stack);
void dlist_back_push(struct dlist *queue, void *value);
void *dlist_back_pop(struct dlist *queue);
void dlist_remove_item(struct dlist *dlist, struct dnode *item);
void dlist_kfree_values(struct dlist *dlist);