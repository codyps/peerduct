#ifndef LIST_H_
#define LIST_H_


#include <stddef.h>

struct list_head {
	struct list_head *prev, *next;
};

#define LIST_HEAD_INIT(x) { .prev = &x, .next = &x }
#define LIST_HEAD(x) struct list_head x = LIST_HEAD_INIT(x)

/* XXX: check that member has type (struct list_head) */
#define container_of(ptr, type, member) \
	({	const typeof( ((type *)NULL)->member ) *mptr__ = ptr;	\
	 	(type *)( (char *)mptr__ - offsetof(type, member)); })

#define list_entry(ptr, type, member) container_of(ptr, type, member)


static inline void _list_add(struct list_head *newl,
			     struct list_head *next,
			     struct list_head *prev)
{
	next->prev = newl;
	prev->next = newl;
	newl->next = next;
	newl->prev = prev;
}

static inline void list_add_tail(struct list_head *newl, struct list_head *head)
{
	_list_add(newl, head, head->prev);
}

/**
 * list_for_each - iterate over a list
 * @head: (struct list_head *) pointer to the head.
 * @curr: (struct list_head *) storage for a *
 *
 */
#define list_for_each(head, curr) for((curr) = (head)->next; (curr) != head; (curr) = (curr)->next)

/**
 * list_for_each_entry - iterate over a list, extracting the entry of each node.
 * @head: (struct list_head *) the sentianl element, lacking an entry. Head of
 *        the list
 * @curr: (type *) pointer to the current list entry, iterator.
 * @type: the type of the list entry
 * @member: name of the member in @type with is the list_head being iterated on
 */
#define list_for_each_entry(head, curr, type, member) \
	for((curr) = list_entry((head)->next, type, member); \
			&((curr)->member) != (head); \
			curr = list_entry(curr->member->next, type, member))

#define list_for_each_entry_safe(head, curr, type, member, store)

#endif
