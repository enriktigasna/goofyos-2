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
struct deque {
	size_t count;
	struct dnode *head;
	struct dnode *tail;
};

void deque_front_push(struct deque *stack, void *value);
struct dnode *deque_front_pop(struct deque *stack);
void deque_back_push(struct deque *queue, void *value);
struct dnode *deque_back_pop(struct deque *queue);