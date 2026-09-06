#ifndef SCC_RBTREE_H
#define SCC_RBTREE_H

#include <scc/arena.h>
#include <scc/mem.h>
#include <scc/pp_token.h>

#include <stddef.h>

#define scc_rbtree(type) type *

#define scc_rbtree_iter(type) type const *

typedef int(*scc_rbcompare)(void const *, void const *);

enum scc_rbcolor {
    scc_rbcolor_black,
    scc_rbcolor_red
};

enum scc_rbdir {
    scc_rbdir_left,
    scc_rbdir_right
};

#define scc_rbnode_link_qual(node, idx, qual)                       \
    (*(struct scc_rbnode_base *qual *)                              \
        ((unsigned char qual *)&(node)->rn_left +                   \
            (scc_rbnode_link_offset(node)) * (idx)))

#define scc_rbnode_link(node, idx)                                  \
    scc_rbnode_link_qual(node, idx,)

#define scc_rbnode_value_qual(base, node, qual)                     \
    ((void qual *)(((unsigned char qual *)node) +                   \
        ((base)->rb_dataoff)))

#define scc_rbnode_value(base, node)                                \
    scc_rbnode_value_qual(base, node,)

struct scc_rbnode_base {
    struct scc_rbnode_base *rn_left;
    struct scc_rbnode_base *rn_right;
    enum scc_rbcolor rn_color;
    unsigned char rn_flags;
    unsigned char rn_data[];
};

struct scc_rbsentinel {
    struct scc_rbnode_base *rs_left;
    struct scc_rbnode_base *rs_right;
    enum scc_rbcolor rs_color;
    unsigned char rs_flags;
};

struct scc_rbtree_base {
    unsigned short rb_dataoff;
    size_t rb_size;
    scc_rbcompare rb_compare;
    struct scc_arena rb_arena;
    struct scc_rbsentinel rb_sentinel;
    unsigned char rb_dynalloc;
    unsigned char rb_fwoff;
    unsigned char rb_data[];
};

#define scc_rbnode_impl_layout(type)                                                        \
    struct {                                                                                \
        struct {                                                                            \
            struct scc_rbnode_base *rn_left;                                                \
            struct scc_rbnode_base *rn_right;                                               \
            enum scc_rbcolor rn_color;                                                      \
            unsigned char rn_flags;                                                         \
            unsigned char rn_bkoff;                                                         \
        } rbn0;                                                                             \
        type rn_value;                                                                      \
    }

#define scc_rbnode_impl_valoff(type)                                                        \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                struct scc_rbnode_base *rn_left;                                            \
                struct scc_rbnode_base *rn_right;                                           \
                enum scc_rbcolor rn_color;                                                  \
                unsigned char rn_flags;                                                     \
                unsigned char rn_bkoff;                                                     \
            } rbn0;                                                                         \
            type rn_value[];                                                                \
        }                                                                                   \
    )

#define scc_rbtree_impl_layout(type)                                                        \
    struct {                                                                                \
        struct {                                                                            \
            unsigned short rb_dataoff;                                                      \
            size_t rb_size;                                                                 \
            scc_rbcompare rb_compare;                                                       \
            struct scc_arena rb_arena;                                                      \
            struct scc_rbsentinel rb_sentinel;                                              \
            unsigned char rb_dynalloc;                                                      \
            unsigned char rb_fwoff;                                                         \
            unsigned char rb_bkoff;                                                         \
        } rb0;                                                                              \
        type rb_curr;                                                                       \
    }

#define scc_rbtree_impl_curroff(type)                                                       \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                unsigned short rb_dataoff;                                                  \
                size_t rb_size;                                                             \
                scc_rbcompare rb_compare;                                                   \
                struct scc_arena rb_arena;                                                  \
                struct scc_rbsentinel rb_sentinel;                                          \
                unsigned char rb_dynalloc;                                                  \
                unsigned char rb_fwoff;                                                     \
                unsigned char rb_bkoff;                                                     \
            } rb0;                                                                          \
            type rb_curr[];                                                                 \
        }                                                                                   \
    )

void *scc_rbtree_impl_new(struct scc_rbtree_base *base, size_t coff);

void *scc_rbtree_impl_new_dyn(size_t treesz, struct scc_arena *arena, scc_rbcompare compare, size_t coff, size_t dataoff);

#define scc_rbtree_new(type, compare)                                                       \
    (type *)scc_rbtree_impl_new(                                                            \
        (void *)&(scc_rbtree_impl_layout(type)) {                                           \
            .rb0 = {                                                                        \
                .rb_dataoff = scc_rbnode_impl_valoff(type),                                 \
                .rb_compare = compare,                                                      \
                .rb_arena = scc_arena_new(scc_rbnode_impl_layout(type)),                    \
            },                                                                              \
        },                                                                                  \
        scc_rbtree_impl_curroff(type)                                                       \
   )

#define scc_rbtree_new_dyn(type, compare)                                                   \
    (type *)scc_rbtree_impl_new_dyn(                                                        \
        sizeof(scc_rbtree_impl_layout(type)),                                               \
        &scc_arena_new(scc_rbnode_impl_layout(type)),                                       \
        compare,                                                                            \
        scc_rbtree_impl_curroff(type),                                                      \
        scc_rbnode_impl_valoff(type)                                                        \
    )

inline size_t scc_rbtree_impl_npad(void const *rbtree) {
    return ((unsigned char const *)rbtree)[-1] + sizeof(unsigned char);
}

inline size_t scc_rbnode_link_offset(struct scc_rbnode_base const *node) {
    return ((unsigned char const*)&node->rn_right) - ((unsigned char const *)&node->rn_left);
}

inline _Bool scc_rbnode_thread(struct scc_rbnode_base const *node, enum scc_rbdir dir) {
    return node->rn_flags & (1u << dir);
}

inline size_t scc_rbnode_bkoff(void const *iter) {
    return ((unsigned char const *)iter)[-1];
}

#define scc_rbnode_impl_base_qual(valaddr, qual)                    \
    scc_container_qual(                                             \
        (unsigned char qual *)valaddr -                             \
            scc_rbnode_bkoff(valaddr),                              \
        struct scc_rbnode_base,                                     \
        rn_data,                                                    \
        qual                                                        \
    )

#define scc_rbnode_impl_base(valaddr)                               \
    scc_rbnode_impl_base_qual(valaddr,)


#define scc_rbtree_impl_base_qual(rbtree, qual)                                             \
    scc_container_qual(                                                                     \
        (unsigned char qual *)(rbtree) - scc_rbtree_impl_npad(rbtree),                      \
        struct scc_rbtree_base,                                                             \
        rb_fwoff,                                                                           \
        qual                                                                                \
    )

#define scc_rbtree_impl_base(rbtree)                                                        \
    scc_rbtree_impl_base_qual(rbtree,)

inline size_t scc_rbtree_size(void const *rbtree) {
    struct scc_rbtree_base const *base = scc_rbtree_impl_base_qual(rbtree, const);
    return base->rb_size;
}

inline _Bool scc_rbtree_empty(void const *rbtree) {
    return !scc_rbtree_size(rbtree);
}

void scc_rbtree_clear(void *rbtree);

void scc_rbtree_free(void *rbtree);

void *scc_rbtree_impl_generic_insert(void *rbtreeaddr, size_t elemsize);

inline _Bool scc_rbtree_impl_insert(void *rbtreeaddr, size_t elemsize) {
    return scc_rbtree_impl_generic_insert(rbtreeaddr, elemsize) == *(void **)rbtreeaddr;
}

#define scc_rbtree_insert(rbtreeaddr, value)                                                \
    scc_rbtree_impl_insert((**(rbtreeaddr) = (value), rbtreeaddr), sizeof(**(rbtreeaddr)))

void const *scc_rbtree_impl_find(void const *rbtree);

#define scc_rbtree_find(rbtree, value)                                                      \
    scc_rbtree_impl_find((*(rbtree) = (value), (rbtree)))

_Bool scc_rbtree_impl_remove(void *rbtree, size_t elemsize);

#define scc_rbtree_remove(rbtree, value)                                                    \
    scc_rbtree_impl_remove((*(rbtree) = (value), (rbtree)), sizeof(*(rbtree)))

void const *scc_rbtree_impl_leftmost_value(void const *rbtree);

void const *scc_rbtree_impl_rightmost_value(void const *rbtree);

void const *scc_rbtree_impl_successor(void const *iter);

void const *scc_rbtree_impl_predecessor(void const *iter);

inline void const *scc_rbtree_impl_iterstop(void const *rbtree) {
    struct scc_rbtree_base const *base = scc_rbtree_impl_base_qual(rbtree, const);
    return (unsigned char const *)&base->rb_sentinel + base->rb_dataoff;
}

void *scc_rbtree_impl_clone(void const *rbtree, size_t elemsize);

#define scc_rbtree_clone(rbtree)                                                            \
    scc_rbtree_impl_clone(rbtree, sizeof(*(rbtree)))

#define scc_rbtree_foreach(iter, rbtree)                                                    \
    for (void const *scc_pp_cat_expand(scc_rbtree_end_,__LINE__) =                           \
            (iter = scc_rbtree_impl_leftmost_value(rbtree),                                 \
                scc_rbtree_impl_iterstop(rbtree));                                          \
        iter != scc_pp_cat_expand(scc_rbtree_end_,__LINE__);                                \
        iter = scc_rbtree_impl_successor(iter))

#define scc_rbtree_foreach_reversed(iter, rbtree)                                           \
    for (void const *scc_pp_cat_expand(scc_rbtree_end_,__LINE__) =                           \
            (iter = scc_rbtree_impl_rightmost_value(rbtree),                                \
                scc_rbtree_impl_iterstop(rbtree));                                          \
        iter != scc_pp_cat_expand(scc_rbtree_end_,__LINE__);                                \
        iter = scc_rbtree_impl_predecessor(iter))

#endif /* SCC_RBTREE_H */
