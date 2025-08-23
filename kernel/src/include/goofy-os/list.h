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
struct dnode *dlist_front_pop(struct dlist *stack);
void dlist_back_push(struct dlist *queue, void *value);
struct dnode *dlist_back_pop(struct dlist *queue);