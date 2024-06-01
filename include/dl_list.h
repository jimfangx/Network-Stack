/*
Implementation of a doubly linked list for skbuff (*next AND *prev
elements) seen in the linux src here:
https://lxr.linux.no/linux+v2.6.20/include/linux/skbuff.h#L184


See this diagram from 61B for the implementation:
https://docs.google.com/presentation/d/1jjivjdvD4mx6qb4bd4rUKB6FDZbYtveJ5NKeyVeA1lk/edit#slide=id.g829fe3f43_0_376

https://github.com/Berkeley-CS61B-Student/sp24-s1085/blob/main/proj1a/src/LinkedListDeque61B.java

*/

#ifndef _LIST_H
#define _LIST_H

#include <stddef.h>

struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

#define LIST_HEAD(name) struct list_head name = {&(name), &(name)}

// define all static inline functions in the header file:
// https://stackoverflow.com/questions/5526461/gcc-warning-function-used-but-not-defined

static inline void list_init(struct list_head *head)
{
    head->next = head;
    head->prev = head;
}

static inline void add_first(struct list_head *new_element, struct list_head *head)
{
    new_element->next = head->next;
    new_element->prev = head;
    head->next->prev = new_element;
    head->next = new_element;
}

static inline void add_last(struct list_head *new_element, struct list_head *head)
{
    new_element->next = head;
    new_element->prev = head->prev;
    head->prev->next = new_element;
    head->prev = new_element;
}

static inline void remove_elem(struct list_head *elem_to_remove)
{
    elem_to_remove->prev->next = elem_to_remove->next;
    elem_to_remove->next->prev = elem_to_remove->prev;
}

static inline int is_list_empty(struct list_head *head)
{
    return head->next == head;
}

#define get_list_item(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#define get_list_first_entry(ptr, type, member) \
    get_list_item((ptr)->next, type, member)

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#endif