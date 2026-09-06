#ifndef SCC_BTREE_H
#define SCC_BTREE_H

#include <scc/arena.h>
#include <scc/bits.h>
#include <scc/bug.h>
#include <scc/mem.h>

#include <stddef.h>

#define scc_btree(type) type *

typedef int(*scc_btcompare)(void const *, void const *);

#ifndef SCC_BTREE_FAULT_ORDER
#define SCC_BTREE_DEFAULT_ORDER 6u
#endif /* SCC_BTREE_DEFAULT_ORDER */

#if SCC_BTREE_DEFAULT_ORDER <= 2
#error Order must be at least 2
#endif

struct scc_btnode_base {
    unsigned char bt_flags;
    unsigned short bt_nkeys;
    unsigned char bt_data[];
};

struct scc_btree_base {
    unsigned short const bt_order;
    unsigned short const bt_dataoff;
    unsigned short const bt_linkoff;
    size_t bt_size;
    struct scc_btnode_base *bt_root;
    scc_btcompare bt_compare;
    struct scc_arena bt_arena;
    unsigned char bt_dynalloc;
    unsigned char bt_fwoff;
    unsigned char bt_data[];
};

#define scc_btnode_impl_layout(type, order)                                                     \
    struct {                                                                                    \
        struct {                                                                                \
            struct {                                                                            \
                unsigned char bt_flags;                                                         \
                unsigned short bt_nkeys;                                                        \
            } btn0;                                                                             \
            type bt_data[(order) - 1u];                                                         \
        } btn1;                                                                                 \
        struct scc_btnode_base *bt_links[order];                                                \
    }

#define scc_btnode_impl_dataoff(type)                                                           \
    sizeof(                                                                                     \
        struct {                                                                                \
            struct {                                                                            \
                unsigned char bt_flags;                                                         \
                unsigned short bt_nkeys;                                                        \
            } btn0;                                                                             \
            type bt_data[];                                                                     \
        }                                                                                       \
    )

#define scc_btnode_impl_linkoff(type, order)                                                    \
    sizeof(                                                                                     \
        struct {                                                                                \
            struct {                                                                            \
                struct {                                                                        \
                    unsigned char bt_flags;                                                     \
                    unsigned short bt_nkeys;                                                    \
                } btn0;                                                                         \
                type bt_data[(order) - 1u];                                                     \
            } btn1;                                                                             \
            struct scc_btnode_base *bt_links[];                                                 \
        }                                                                                       \
    )

#define scc_btree_impl_layout(type, order)                                                          \
    struct {                                                                                        \
        struct {                                                                                    \
            struct {                                                                                \
                unsigned short const bt_order;                                                      \
                unsigned short const bt_dataoff;                                                    \
                unsigned short const bt_linkoff;                                                    \
                size_t bt_size;                                                                     \
                struct scc_btnode_base *bt_root;                                                    \
                scc_btcompare bt_compare;                                                           \
                struct scc_arena bt_arena;                                                          \
                unsigned char bt_dynalloc;                                                          \
                unsigned char bt_fwoff;                                                             \
                unsigned char bt_bkoff;                                                             \
            } bt0;                                                                                  \
            type bt_curr;                                                                           \
        } bt1;                                                                                      \
        scc_btnode_impl_layout(type, order) bt_rootmem;                                             \
    }

#define scc_btree_impl_curroff(type)                                                                \
    sizeof(                                                                                         \
        struct {                                                                                    \
            struct {                                                                                \
                unsigned short const bt_order;                                                      \
                unsigned short const bt_dataoff;                                                    \
                unsigned short const bt_linkoff;                                                    \
                size_t bt_size;                                                                     \
                struct scc_btnode_base *bt_root;                                                    \
                scc_btcompare bt_compare;                                                           \
                struct scc_arena bt_arena;                                                          \
                unsigned char bt_dynalloc;                                                          \
                unsigned char bt_fwoff;                                                             \
                unsigned char bt_bkoff;                                                             \
            } bt0;                                                                                  \
            type bt_curr[];                                                                         \
        }                                                                                           \
    )

#define scc_btree_impl_rootoff(type, order)                                                         \
    sizeof(                                                                                         \
        struct {                                                                                    \
            struct {                                                                                \
                struct {                                                                            \
                    unsigned short const bt_order;                                                  \
                    unsigned short const bt_dataoff;                                                \
                    unsigned short const bt_linkoff;                                                \
                    size_t bt_size;                                                                 \
                    struct scc_btnode_base *bt_root;                                                \
                    scc_btcompare bt_compare;                                                       \
                    struct scc_arena bt_arena;                                                      \
                    unsigned char bt_dynalloc;                                                      \
                    unsigned char bt_fwoff;                                                         \
                    unsigned char bt_bkoff;                                                         \
                } bt0;                                                                              \
                type bt_curr;                                                                       \
            } bt1;                                                                                  \
            scc_btnode_impl_layout(type, order) bt_rootmem[];                                       \
        }                                                                                           \
    )


void *scc_btree_impl_new(void *base, size_t coff, size_t rootoff);

void *scc_btree_impl_new_dyn(void *sbase, size_t basesz, size_t coff, size_t rootoff);

inline void *scc_btree_impl_with_order(void *base, size_t coff, size_t rootoff) {
    unsigned order = ((struct scc_btree_base *)base)->bt_order;
    if (order < 3u) {
        return 0;
    }
    return scc_btree_impl_new(base, coff, rootoff);
}

inline void *scc_btree_impl_with_order_dyn(void *sbase, size_t basesz, size_t coff, size_t rootoff) {
    unsigned order = ((struct scc_btree_base *)sbase)->bt_order;
    if (order < 3u) {
        return 0;
    }
    return scc_btree_impl_new_dyn(sbase, basesz, coff, rootoff);
}

#define scc_btree_with_order(type, compare, order)                                                  \
    scc_btree_impl_with_order(&(scc_btree_impl_layout(type, order)) {                               \
            .bt1 = {                                                                                \
                .bt0 = {                                                                            \
                    .bt_order = order,                                                              \
                    .bt_dataoff = scc_btnode_impl_dataoff(type),                                    \
                    .bt_linkoff = scc_btnode_impl_linkoff(type, order),                             \
                    .bt_arena = scc_arena_new(scc_btnode_impl_layout(type, order)),                 \
                    .bt_compare = compare                                                           \
                },                                                                                  \
            },                                                                                      \
        },                                                                                          \
        scc_btree_impl_curroff(type),                                                               \
        scc_btree_impl_rootoff(type, order)                                                         \
    )

#define scc_btree_new(type, compare)                                                                \
    (type *)scc_btree_impl_new(&(scc_btree_impl_layout(type, SCC_BTREE_DEFAULT_ORDER)) {            \
            .bt1 = {                                                                                \
                .bt0 = {                                                                            \
                    .bt_order = SCC_BTREE_DEFAULT_ORDER,                                            \
                    .bt_dataoff = scc_btnode_impl_dataoff(type),                                    \
                    .bt_linkoff = scc_btnode_impl_linkoff(type, SCC_BTREE_DEFAULT_ORDER),           \
                    .bt_arena = scc_arena_new(                                                      \
                        scc_btnode_impl_layout(type, SCC_BTREE_DEFAULT_ORDER)                       \
                    ),                                                                              \
                    .bt_compare = (compare)                                                         \
                },                                                                                  \
            },                                                                                      \
        },                                                                                          \
        scc_btree_impl_curroff(type),                                                               \
        scc_btree_impl_rootoff(type, SCC_BTREE_DEFAULT_ORDER)                                       \
    )

#define scc_btree_with_order_dyn(type, compare, order)                                              \
    scc_btree_impl_with_order_dyn(&(scc_btree_impl_layout(type, order)) {                           \
            .bt1 = {                                                                                \
                .bt0 = {                                                                            \
                    .bt_order = (order),                                                            \
                    .bt_dataoff = scc_btnode_impl_dataoff(type),                                    \
                    .bt_linkoff = scc_btnode_impl_linkoff(type, order),                             \
                    .bt_arena = scc_arena_new(scc_btnode_impl_layout(type, (order))),               \
                    .bt_compare = (compare)                                                         \
                },                                                                                  \
            },                                                                                      \
        },                                                                                          \
        sizeof(scc_btree_impl_layout(type, order)),                                                 \
        scc_btree_impl_curroff(type),                                                               \
        scc_btree_impl_rootoff(type, order)                                                         \
    )

#define scc_btree_new_dyn(type, compare)                                                            \
    (type *)scc_btree_impl_new_dyn(&(scc_btree_impl_layout(type, SCC_BTREE_DEFAULT_ORDER)) {        \
            .bt1 = {                                                                                \
                .bt0 = {                                                                            \
                    .bt_order = SCC_BTREE_DEFAULT_ORDER,                                            \
                    .bt_dataoff = scc_btnode_impl_dataoff(type),                                    \
                    .bt_linkoff = scc_btnode_impl_linkoff(type, SCC_BTREE_DEFAULT_ORDER),           \
                    .bt_arena = scc_arena_new(                                                      \
                        scc_btnode_impl_layout(type, SCC_BTREE_DEFAULT_ORDER)                       \
                    ),                                                                              \
                    .bt_compare = (compare)                                                         \
                },                                                                                  \
            },                                                                                      \
        },                                                                                          \
        sizeof(scc_btree_impl_layout(type, SCC_BTREE_DEFAULT_ORDER)),                               \
        scc_btree_impl_curroff(type),                                                               \
        scc_btree_impl_rootoff(type, SCC_BTREE_DEFAULT_ORDER)                                       \
    )

inline size_t scc_btree_impl_npad(void const *btree) {
    return ((unsigned char const *)btree)[-1] + sizeof(unsigned char);
}

#define scc_btree_impl_base_qual(btree, qual)                                                       \
    scc_container_qual(                                                                             \
        (unsigned char qual *)(btree) - scc_btree_impl_npad(btree),                                 \
        struct scc_btree_base,                                                                      \
        bt_fwoff,                                                                                   \
        qual                                                                                        \
    )

#define scc_btree_impl_base(btree)                                                                  \
    scc_btree_impl_base_qual(btree,)

void scc_btree_free(void *btree);

inline size_t scc_btree_order(void const *btree) {
    struct scc_btree_base const *base = scc_btree_impl_base_qual(btree, const);
    return base->bt_order;
}

inline size_t scc_btree_size(void const *btree) {
    struct scc_btree_base const *base = scc_btree_impl_base_qual(btree, const);
    return base->bt_size;
}

_Bool scc_btree_impl_insert(void *btreeaddr, size_t elemsize);

#define scc_btree_insert(btreeaddr, value)                                                              \
    scc_btree_impl_insert((**(btreeaddr) = (value), btreeaddr), sizeof(**(btreeaddr)))

void const *scc_btree_impl_find(void const *btree, size_t elemsize);

#define scc_btree_find(btree, value)                                                                    \
    scc_btree_impl_find((*(btree) = (value), (btree)), sizeof(*(btree)))

_Bool scc_btree_impl_remove(void *btree, size_t elemsize);

#define scc_btree_remove(btree, value)                                                                  \
    scc_btree_impl_remove(((*(btree) = (value)), (btree)), sizeof(*(btree)))

void *scc_btree_impl_clone(void const *btree, size_t elemsize);

#define scc_btree_clone(btree)                                                                              \
    scc_btree_impl_clone(btree, sizeof(*(btree)))

#endif /* SCC_BTREE_H */
