#pragma once
#include <stddef.h>
#include <stdint.h>

struct dnode {
	void *value;
	struct dnode *next;
	struct dnode *prev;
};

struct list_head {
	struct list_head *next;
	struct list_head *prev;
};

struct single_list_head {
	struct single_list_head *next;
};

struct list_head *list_pop_front(struct list_head **head);
void list_push_front(struct list_head **head, struct list_head *needle);
void list_remove_node(struct list_head **head, struct list_head *needle);
struct single_list_head *slist_pop_front(struct single_list_head **head);
void slist_push_front(struct single_list_head **head,
		      struct single_list_head *needle);

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