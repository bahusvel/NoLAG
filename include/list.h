#ifndef __MLIST__
#define __MLIST__

#define LIST_INSERT(item_ptr, list)                                            \
	item_ptr->next = list.next;                                                \
	list.next = item_ptr

#define LIST_FOREACH(iter_ptr, list)                                           \
	for (iter_ptr = list.next; iter_ptr != NULL; iter_ptr = iter_ptr->next)

#define LIST_DELETE(item_ptr, list)                                            \
	if (list.next == item_ptr)                                                 \
		list.next = list.next->next;                                           \
	__typeof__(item_ptr) iter_ptr;                                             \
	for (iter_ptr = list.next; iter_ptr->next != NULL;                         \
		 iter_ptr = iter_ptr->next) {                                          \
		if (iter_ptr->next == item_ptr)                                        \
			iter_ptr->next = iter_ptr->next->next;                             \
	}

#endif
