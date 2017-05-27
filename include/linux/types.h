#ifndef _TYPES_H_
#define _TYPES_H_

struct list_head {
	struct list_head *prev;
	struct list_head *next;
};

#endif
