/*
 * Copyright (c) 2022 Vilhelm Engström
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SCC_RBMAP_H
#define SCC_RBMAP_H

#include <scc/rbtree.h>

#include <stddef.h>

#define scc_rbmap_impl_pair(keytype, valuetype)                                             \
    struct { keytype rm_key; valuetype rm_value; }

#define scc_rbmap(keytype, valuetype)                                                       \
    scc_rbmap_impl_pair(keytype, valuetype) *

#define scc_rbmap_iter(keytype, valuetype)                                                  \
    struct { keytype const key; valuetype value; } *

typedef int(*scc_rmcompare)(void const *, void const *);

#define scc_rbmnode_impl_layout(keytype, valuetype) \
    struct {                                                                                \
        struct {                                                                            \
            struct scc_rbnode_base *rn_left;                                                \
            struct scc_rbnode_base *rn_right;                                               \
            enum scc_rbcolor rn_color;                                                      \
            unsigned char rn_flags;                                                         \
            unsigned char rn_bkoff;                                                         \
        } rbmn0;                                                                            \
        scc_rbmap_impl_pair(keytype, valuetype) rn_pair;                                    \
    }

#define scc_rbmnode_impl_pairoff(keytype, valuetype)                                        \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                struct scc_rbnode_base *rn_left;                                            \
                struct scc_rbnode_base *rn_right;                                           \
                enum scc_rbcolor rn_color;                                                  \
                unsigned char rn_flags;                                                     \
                unsigned char rn_bkoff;                                                     \
            } rbmn0;                                                                        \
            scc_rbmap_impl_pair(keytype, valuetype) rn_pair[];                              \
        }                                                                                   \
    )

#define scc_rbmap_impl_layout(keytype, valuetype)                                           \
    struct {                                                                                \
        struct {                                                                            \
            unsigned short const rm_dataoff;                                                \
            size_t rm_size;                                                                 \
            scc_rmcompare rm_compare;                                                       \
            struct scc_arena rm_arena;                                                      \
            struct scc_rbsentinel rm_sentinel;                                              \
            unsigned char rm_fwoff;                                                         \
            unsigned char rm_bkoff;                                                         \
        } rbm0;                                                                             \
        scc_rbmap_impl_pair(keytype, valuetype) rm_curr;                                    \
    }

#define scc_rbmap_impl_curroff(keytype, valuetype)                                          \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                unsigned short const rm_dataoff;                                            \
                size_t rm_size;                                                             \
                scc_rmcompare rm_compare;                                                   \
                struct scc_arena rm_arena;                                                  \
                struct scc_rbsentinel rm_sentinel;                                          \
                unsigned char rm_fwoff;                                                     \
                unsigned char rm_bkoff;                                                     \
            } rbm0;                                                                         \
            scc_rbmap_impl_pair(keytype, valuetype) rm_curr[];                              \
        }                                                                                   \
    )

#define scc_rbmap_new(keytype, valuetype, compare)                                          \
    scc_rbtree_impl_new(                                                                    \
        (void *)&(scc_rbmap_impl_layout(keytype, valuetype)) {                              \
            .rbm0 = {                                                                       \
                .rm_dataoff = scc_rbmnode_impl_pairoff(keytype, valuetype),                 \
                .rm_compare = compare,                                                      \
                .rm_arena = scc_arena_new(scc_rbmnode_impl_layout(keytype, valuetype)),     \
            },                                                                              \
        },                                                                                  \
        scc_rbmap_impl_curroff(keytype, valuetype)                                          \
   )

#define scc_rbmap_new_dyn(keytype, valuetype, compare)                                      \
    scc_rbtree_impl_new_dyn(                                                                \
        sizeof(scc_rbmap_impl_layout(keytype, valuetype)),                                  \
        &scc_arena_new(scc_rbmnode_impl_layout(keytype, valuetype)),                        \
        compare,                                                                            \
        scc_rbmap_impl_curroff(keytype, valuetype),                                         \
        scc_rbmnode_impl_pairoff(keytype, valuetype)                                        \
    )

inline size_t scc_rbmap_size(void const *map) {
    return scc_rbtree_size(map);
}

inline _Bool scc_rbmap_empty(void const *map) {
    return !scc_rbmap_size(map);
}

inline void scc_rbmap_clear(void *map) {
    scc_rbtree_clear(map);
}

inline void scc_rbmap_free(void *map) {
    scc_rbtree_free(map);
}

_Bool scc_rbmap_impl_insert(void *rbmapaddr, size_t elemsize, size_t valoff);

#define scc_rbmap_insert(mapaddr, key, value)                                               \
    scc_rbmap_impl_insert((                                                                 \
            (*(mapaddr))->rm_key = (key),                                                   \
            (*(mapaddr))->rm_value = (value),                                               \
            (mapaddr)                                                                       \
        ),                                                                                  \
        sizeof(**(mapaddr)),                                                                \
        ((unsigned char const *)&(*(mapaddr))->rm_value -                                   \
            (unsigned char const *)&(*(mapaddr))->rm_key)                                   \
    )

void *scc_rbmap_impl_find(void *map, size_t valoff);

#define scc_rbmap_find(map, key)                                                            \
    scc_rbmap_impl_find(                                                                    \
        ((map)->rm_key = (key), (map)),                                                     \
        ((unsigned char const *)&(map)->rm_value - (unsigned char const *)&(map)->rm_key)   \
    )

#define scc_rbmap_remove(map, key)                                                          \
    scc_rbtree_impl_remove(                                                                 \
        ((map)->rm_key = (key), (map)),                                                     \
        sizeof(*(map))                                                                      \
    )

void *scc_rbmap_impl_leftmost_pair(void *map);

void *scc_rbmap_impl_rightmost_pair(void *map);

void *scc_rbmap_impl_successor(void *iter);

void *scc_rbmap_impl_predecessor(void *iter);

inline void const *scc_rbmap_impl_iterstop(void const *map) {
    return scc_rbtree_impl_iterstop(map);
}

#define scc_rbmap_clone(map)                                                                \
    scc_rbtree_impl_clone(map, sizeof(*(map)))

#define scc_rbmap_foreach(iter, map)                                                        \
    for (void const *scc_pp_cat_expand(scc_rbmap_end_,__LINE__) =                            \
            ((iter) = scc_rbmap_impl_leftmost_pair(map),                                    \
                scc_rbmap_impl_iterstop(map));                                              \
        (iter) != scc_pp_cat_expand(scc_rbmap_end_,__LINE__);                               \
        (iter) = scc_rbmap_impl_successor(iter))

#define scc_rbmap_foreach_reversed(iter, rbmap)                                             \
    for (void const *scc_pp_cat_expand(scc_rbmap_end_,__LINE__) =                            \
            (iter = scc_rbmap_impl_rightmost_pair(rbmap),                                   \
                scc_rbmap_impl_iterstop(rbmap));                                            \
        iter != scc_pp_cat_expand(scc_rbmap_end_,__LINE__);                                 \
        iter = scc_rbmap_impl_predecessor(iter))

#endif /* SCC_RBMAP_H */
