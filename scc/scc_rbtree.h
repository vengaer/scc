#ifndef SCC_RBTREE_H
#define SCC_RBTREE_H

#include "scc_arena.h"

#include <stddef.h>

typedef int(*scc_rbcompare)(void const *, void const *);

enum scc_rbcolor {
    scc_rbcolor_black,
    scc_rbcolor_red
};

struct scc_rbnode_bare {
    union {
        struct {
            struct scc_rbnode_bare *left;   /* Left link */
            struct scc_rbnode_bare *right;  /* Right link */
        } node;
        struct {
            struct scc_rbtree *parent;      /* Address of tree */
        } temp;
    } un_link;
    enum scc_rbcolor color;
    unsigned char flags;                    /* Thread flags */
    unsigned char pad;                      /* Explicit padding */
};

struct scc_rbnode {
    struct scc_rbnode_bare rn_bare;
    unsigned char rn_buffer[];
};

#define scc_rbnode_impl_layout(type)                                \
    struct {                                                        \
        struct scc_rbnode_bare rn_bare;                             \
        type rn_value;                                              \
    }

#define scc_rbtree_impl_baseoff(type)                               \
    offsetof(scc_rbnode_impl_layout(type), rn_value)

struct scc_rbtree {
    unsigned short rb_baseoff;
    size_t rb_size;
    struct scc_rbnode_bare rb_sentinel;
    struct scc_arena rb_arena;
    scc_rbcompare rb_compare;
};

void *scc_rbtree_impl_init(struct scc_rbtree *restrict tree);
_Bool scc_rbtree_impl_insert(void *handle);
void const *scc_rbtree_impl_find(void *handle);

#define scc_rbtree_init(type, compare)                              \
    scc_rbtree_impl_init(&(struct scc_rbtree) {                     \
        .rb_baseoff = scc_rbtree_impl_baseoff(type),                \
        .rb_arena = scc_arena_init(scc_rbnode_impl_layout(type)),   \
        .rb_compare = compare                                       \
    })

void scc_rbtree_free(void *addr);
size_t scc_rbtree_size(void const *handle);

inline _Bool scc_rbtree_empty(void const *handle) {
    return !scc_rbtree_size(handle);
}

#define scc_rbtree_insert(handle, value)                            \
    scc_rbtree_impl_insert((*handle = value, &handle))

#define scc_rbtree_find(handle, value)                              \
    scc_rbtree_impl_find((*handle = value, handle))

#endif /* SCC_RBTREE_H */
