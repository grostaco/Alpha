#ifndef __LINKED_H
#define __LINKED_H

#include <stddef.h>

#define container_of(t,p,d) (t*)((char*)p - (char*)offsetof(t,d))

struct slist_node {
    struct slist_node* next;
};

#define slist_set_next(ptr,entry) ({                                         \
    const typeof(ptr) __next_entry = (entry);                                \
    struct slist_node*    __curr = (struct slist_node*)&(ptr)->next;         \
    struct slist_node*    __next = __next_entry ?                            \
                                   (struct slist_node*)&__next_entry->next : \
                                   NULL;                                     \
    __curr->next = __next;                                                   \
})

#define slist_set_next_explicit(ptr,field,entry) ({                           \
    const typeof(ptr) __next_entry = (entry);                                 \
    struct slist_node*    __curr = (struct slist_node*)&(ptr)->field;         \
    struct slist_node*    __next = __next_entry ?                             \
                                   (struct slist_node*)&__next_entry->field : \
                                   NULL;                                      \
    __curr->field = __next;                                                   \
})


#define slist_get_next(ptr) ({                   \
    !(ptr)->next ?                               \
    NULL          :                              \
    container_of(typeof(*ptr),(ptr)->next,next); \
})

#define slist_get_next_explicit(ptr,field) ({     \
    !(ptr)->field ?                               \
    NULL          :                               \
    container_of(typeof(*ptr),(ptr)->next,field); \
})

#define slist_append(head,entry) ({                                   \
    struct slist_node* __iter = (struct slist_node*)&(head)->next;    \
    while(__iter->next)__iter=__iter->next;                           \
    slist_set_next(container_of(typeof(*head), __iter, next), entry); \
})

#define slist_append_explicit(head,field,entry) ({                                     \
    struct slist_node* __iter = (struct slist_node*)&(head)->field;                    \
    while(__iter->field)__iter=__iter->field;                                          \
    slist_set_next_explicit(container_of(typeof(*head), __iter, field), field, entry); \
})

#define slist_foreach(head, unary_function) ({                     \
    struct slist_node* __curr = (head)->next;                      \
    unary_function(head);                                          \
    while(__curr) {                                                \
        unary_function(container_of(typeof(*head), __curr, next)); \
        __curr=__curr->next;                                       \
    }                                                              \
})

#define slist_foreach_explicit(head, field, unary_function) ({      \
    struct slist_node* __curr = (head)->field;                      \
    unary_function(head);                                           \
    while(__curr) {                                                 \
        unary_function(container_of(typeof(*head), __curr, field)); \
        __curr=__curr->field;                                       \
    }                                                               \
})

// if time calls for it, implement this
struct dlist_node {
    struct dlist_node* prev, *next;
};

#endif