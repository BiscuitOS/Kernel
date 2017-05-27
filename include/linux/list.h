#ifndef __LIST_H__
#define __LIST_H__

#include <linux/types.h>

#define LIST_HEAD(name) \
		struct list_head name = {.pre = &(name), .next = &(name)}

#define LIST_HEAD_INIT(head) \
	do {	\
		head->pre = head;	\
		head->next = head;	\
	} while (0)

static inline void list_init(struct list_head *head)
{
	head->next = head;
	head->pre = head;
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	new->next = head;
	new->pre = head->pre;
	head->pre->next = new;
	head->pre = new;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
	new->next = head->next;
	new->pre = head;
	head->next->pre = new;
	head->next = new;
}

static inline void list_del(struct list_head *entry)
{
	struct list_head *pre = entry->pre;
	struct list_head *next = entry->next;

	pre->next = next;
	next->pre = pre;

	entry->pre = 0;
	entry->next = 0;
}

#endif
