#ifndef __LIST_H__
#define __LIST_H__

#include <linux/types.h>

#define LIST_HEAD(name) \
		struct list_head name = {.prev = &(name), .next = &(name)}

#define LIST_HEAD_INIT(head) \
	do {	\
		head->prev = head;	\
		head->next = head;	\
	} while (0)

static inline void list_init(struct list_head *head)
{
	head->next = head;
	head->prev = head;
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	new->next = head;
	new->prev = head->prev;
	head->prev->next = new;
	head->prev = new;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
	new->next = head->next;
	new->prev = head;
	head->next->prev = new;
	head->next = new;
}

static inline void list_del(struct list_head *entry)
{
	struct list_head *prev = entry->prev;
	struct list_head *next = entry->next;

	prev->next = next;
	next->prev = prev;

	entry->prev = 0;
	entry->next = 0;
}

#endif
