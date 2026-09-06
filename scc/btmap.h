#ifndef SCC_BTMAP_H
#define SCC_BTMAP_H

#include <scc/arena.h>
#include <scc/btree.h>
#include <scc/mem.h>

#include <stddef.h>

#define scc_btmap_impl_pair(keytype, valuetype)                                                                 \
    struct { keytype btm_key; valuetype btm_value; }

#define scc_btmap_impl_pair_valoff(keytype, valuetype)                                                          \
    scc_align(sizeof(keytype), scc_alignof(valuetype))

#define scc_btmap(keytype, valuetype)                                                                           \
    scc_btmap_impl_pair(keytype, valuetype) *

typedef scc_btcompare scc_btmcompare;

#ifndef SCC_BTMAP_DEFAULT_ORDER
#define SCC_BTMAP_DEFAULT_ORDER SCC_BTREE_DEFAULT_ORDER
#endif /* SCC_BTMAP_DEFAULT_ORDER */

struct scc_btmnode_base {
    unsigned char btm_flags;
    unsigned short btm_nkeys;
    unsigned char btm_data[];
};

struct scc_btmap_base {
    unsigned short const btm_order;
    unsigned short const btm_keyoff;
    unsigned short const btm_valoff;
    unsigned short const btm_linkoff;
    unsigned short const btm_keysize;
    unsigned short const btm_valsize;
    size_t btm_size;
    struct scc_btmnode_base *btm_root;
    scc_btmcompare btm_compare;
    struct scc_arena btm_arena;
    unsigned char const btm_kvoff;
    unsigned char btm_dynalloc;
    unsigned char btm_fwoff;
    unsigned char btm_data[];
};

#define scc_btmnode_impl_layout(keytype, valuetype, order)                                                          \
    struct {                                                                                                        \
        struct {                                                                                                    \
            struct {                                                                                                \
                struct {                                                                                            \
                    unsigned char btm_flags;                                                                        \
                    unsigned short btm_nkeys;                                                                       \
                } btmn0;                                                                                            \
                keytype btm_keys[(order) - 1u];                                                                     \
            } btmn1;                                                                                                \
            valuetype btm_vals[(order) - 1u];                                                                       \
        } btmn2;                                                                                                    \
        struct scc_btmnode_base *btm_links[order];                                                                  \
    }

#define scc_btmnode_impl_keyoff(keytype)                                                                            \
    sizeof(                                                                                                         \
        struct {                                                                                                    \
            struct {                                                                                                \
                unsigned char btm_flags;                                                                            \
                unsigned short btm_nkeys;                                                                           \
            } btmn0;                                                                                                \
            keytype btm_keys[];                                                                                     \
        }                                                                                                           \
    )

#define scc_btmnode_impl_valoff(keytype, valuetype, order)                                                          \
    sizeof(                                                                                                         \
        struct {                                                                                                    \
            struct {                                                                                                \
                struct {                                                                                            \
                    unsigned char btm_flags;                                                                        \
                    unsigned short btm_nkeys;                                                                       \
                } btmn0;                                                                                            \
                keytype btm_keys[(order) - 1u];                                                                     \
            } btmn1;                                                                                                \
            valuetype btm_vals[];                                                                                   \
        }                                                                                                           \
    )

#define scc_btmnode_impl_linkoff(keytype, valuetype, order)                                                         \
    sizeof(                                                                                                         \
        struct {                                                                                                    \
            struct {                                                                                                \
                struct {                                                                                            \
                    struct {                                                                                        \
                        unsigned char btm_flags;                                                                    \
                        unsigned short btm_Nkeys;                                                                   \
                    } btmn0;                                                                                        \
                    keytype btm_keys[(order) - 1u];                                                                 \
                } btmn1;                                                                                            \
                valuetype btm_vals[(order) - 1u];                                                                   \
            } btmn2;                                                                                                \
            struct scc_btmnode_base *btm_links[];                                                                   \
        }                                                                                                           \
    )

#define scc_btmap_impl_layout(keytype, valuetype, order)                                                                \
    struct {                                                                                                            \
        struct {                                                                                                        \
            struct {                                                                                                    \
                unsigned short const btm_order;                                                                         \
                unsigned short const btm_keyoff;                                                                        \
                unsigned short const btm_valoff;                                                                        \
                unsigned short const btm_linkoff;                                                                       \
                unsigned short const btm_keysize;                                                                       \
                unsigned short const btm_valsize;                                                                       \
                size_t btm_size;                                                                                        \
                struct scc_btmnode_base *btm_root;                                                                      \
                scc_btmcompare btm_compare;                                                                             \
                struct scc_arena btm_arena;                                                                             \
                unsigned char const btm_kvoff;                                                                          \
                unsigned char btm_dynalloc;                                                                             \
                unsigned char btm_fwoff;                                                                                \
                unsigned char btm_bkoff;                                                                                \
            } btm0;                                                                                                     \
            scc_btmap_impl_pair(keytype, valuetype) btm_curr;                                                           \
        } btm1;                                                                                                         \
        scc_btmnode_impl_layout(keytype, valuetype,order) btm_rootmem;                                                  \
    }

#define scc_btmap_impl_curroff(keytype, valuetype, order)                                                               \
    sizeof(                                                                                                             \
        struct {                                                                                                        \
            struct {                                                                                                    \
                unsigned short const btm_order;                                                                         \
                unsigned short const btm_keyoff;                                                                        \
                unsigned short const btm_valoff;                                                                        \
                unsigned short const btm_linkoff;                                                                       \
                unsigned short const btm_keysize;                                                                       \
                unsigned short const btm_valsize;                                                                       \
                size_t btm_size;                                                                                        \
                struct scc_btmnode_base *btm_root;                                                                      \
                scc_btmcompare btm_compare;                                                                             \
                struct scc_arena btm_arena;                                                                             \
                unsigned char const btm_kvoff;                                                                          \
                unsigned char btm_dynalloc;                                                                             \
                unsigned char btm_fwoff;                                                                                \
                unsigned char btm_bkoff;                                                                                \
            } btmn0;                                                                                                    \
            scc_btmap_impl_pair(keytype, valuetype) btm_curr[];                                                         \
        }                                                                                                               \
    )

#define scc_btmap_impl_rootoff(keytype, valuetype, order)                                                               \
    sizeof(                                                                                                             \
        struct {                                                                                                        \
            struct {                                                                                                    \
                struct {                                                                                                \
                    unsigned short const btm_order;                                                                     \
                    unsigned short const btm_keyoff;                                                                    \
                    unsigned short const btm_valoff;                                                                    \
                    unsigned short const btm_linkoff;                                                                   \
                    unsigned short const btm_keysize;                                                                   \
                    unsigned short const btm_valsize;                                                                   \
                    size_t btm_size;                                                                                    \
                    struct scc_btmnode_base *btm_root;                                                                  \
                    scc_btmcompare btm_compare;                                                                         \
                    struct scc_arena btm_arena;                                                                         \
                    unsigned char const btm_kvoff;                                                                      \
                    unsigned char btm_dynalloc;                                                                         \
                    unsigned char btm_fwoff;                                                                            \
                    unsigned char btm_bkoff;                                                                            \
                } btm0;                                                                                                 \
                scc_btmap_impl_pair(keytype, valuetype) btm_curr;                                                       \
            } btm1;                                                                                                     \
            scc_btmnode_impl_layout(keytype, valuetype,order) btm_rootmem[];                                            \
        }                                                                                                               \
    )

#define scc_btmap_with_order(keytype, valuetype, compare, order)                                                        \
    scc_btmap_impl_with_order(&(scc_btmap_impl_layout(keytype, valuetype, order)) {                                     \
            .btm1 = {                                                                                                   \
                .btm0 = {                                                                                               \
                    .btm_order = order,                                                                                 \
                    .btm_keyoff = scc_btmnode_impl_keyoff(keytype),                                                     \
                    .btm_valoff = scc_btmnode_impl_valoff(keytype, valuetype, order),                                   \
                    .btm_linkoff = scc_btmnode_impl_linkoff(keytype, valuetype, order),                                 \
                    .btm_keysize = sizeof(keytype),                                                                     \
                    .btm_valsize = sizeof(valuetype),                                                                   \
                    .btm_arena = scc_arena_new(scc_btmnode_impl_layout(keytype, valuetype, order)),                     \
                    .btm_compare = compare,                                                                             \
                    .btm_kvoff = scc_btmap_impl_pair_valoff(keytype, valuetype)                                         \
                },                                                                                                      \
            },                                                                                                          \
        },                                                                                                              \
        scc_btmap_impl_curroff(keytype, valuetype, order),                                                              \
        scc_btmap_impl_rootoff(keytype, valuetype, order)                                                               \
    )

#define scc_btmap_new(keytype, valuetype, compare)                                                                      \
    scc_btmap_impl_new(&(scc_btmap_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER)) {                          \
            .btm1 = {                                                                                                   \
                .btm0 = {                                                                                               \
                    .btm_order = SCC_BTMAP_DEFAULT_ORDER,                                                               \
                    .btm_keyoff = scc_btmnode_impl_keyoff(keytype),                                                     \
                    .btm_valoff = scc_btmnode_impl_valoff(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER),                 \
                    .btm_linkoff = scc_btmnode_impl_linkoff(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER),               \
                    .btm_keysize = sizeof(keytype),                                                                     \
                    .btm_valsize = sizeof(valuetype),                                                                   \
                    .btm_arena = scc_arena_new(scc_btmnode_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER)),   \
                    .btm_compare = compare,                                                                             \
                    .btm_kvoff = scc_btmap_impl_pair_valoff(keytype, valuetype)                                         \
                },                                                                                                      \
            },                                                                                                          \
        },                                                                                                              \
        scc_btmap_impl_curroff(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER),                                            \
        scc_btmap_impl_rootoff(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER)                                             \
    )

#define scc_btmap_with_order_dyn(keytype, valuetype, compare, order)                                                    \
    scc_btmap_impl_with_order_dyn(&(scc_btmap_impl_layout(keytype, valuetype, order)) {                                 \
            .btm1 = {                                                                                                   \
                .btm0 = {                                                                                               \
                    .btm_order = order,                                                                                 \
                    .btm_keyoff = scc_btmnode_impl_keyoff(keytype),                                                     \
                    .btm_valoff = scc_btmnode_impl_valoff(keytype, valuetype, order),                                   \
                    .btm_linkoff = scc_btmnode_impl_linkoff(keytype, valuetype, order),                                 \
                    .btm_keysize = sizeof(keytype),                                                                     \
                    .btm_valsize = sizeof(valuetype),                                                                   \
                    .btm_arena = scc_arena_new(scc_btmnode_impl_layout(keytype, valuetype, order)),                     \
                    .btm_compare = compare,                                                                             \
                    .btm_kvoff = scc_btmap_impl_pair_valoff(keytype, valuetype)                                         \
                },                                                                                                      \
            },                                                                                                          \
        },                                                                                                              \
        sizeof(scc_btmap_impl_layout(keytype, valuetype, order)),                                                       \
        scc_btmap_impl_curroff(keytype, valuetype, order),                                                              \
        scc_btmap_impl_rootoff(keytype, valuetype, order)                                                               \
    )

#define scc_btmap_new_dyn(keytype, valuetype, compare)                                                                  \
    scc_btmap_impl_new_dyn(&(scc_btmap_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER)) {                      \
            .btm1 = {                                                                                                   \
                .btm0 = {                                                                                               \
                    .btm_order = SCC_BTMAP_DEFAULT_ORDER,                                                               \
                    .btm_keyoff = scc_btmnode_impl_keyoff(keytype),                                                     \
                    .btm_valoff = scc_btmnode_impl_valoff(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER),                 \
                    .btm_linkoff = scc_btmnode_impl_linkoff(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER),               \
                    .btm_keysize = sizeof(keytype),                                                                     \
                    .btm_valsize = sizeof(valuetype),                                                                   \
                    .btm_arena = scc_arena_new(scc_btmnode_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER)),   \
                    .btm_compare = compare,                                                                             \
                    .btm_kvoff = scc_btmap_impl_pair_valoff(keytype, valuetype)                                         \
                },                                                                                                      \
            },                                                                                                          \
        },                                                                                                              \
        sizeof(scc_btmap_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER)),                                     \
        scc_btmap_impl_curroff(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER),                                            \
        scc_btmap_impl_rootoff(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER)                                             \
    )

void *scc_btmap_impl_new(void *base, size_t coff, size_t rootoff);

void *scc_btmap_impl_new_dyn(void *sbase, size_t basesz, size_t coff, size_t rootoff);

inline void *scc_btmap_impl_with_order(void *base, size_t coff, size_t rootoff) {
    unsigned order = ((struct scc_btmap_base *)base)->btm_order;
    if (order < 3u) {
        return 0;
    }
    return scc_btmap_impl_new(base, coff, rootoff);
}

inline void *scc_btmap_impl_with_order_dyn(void *sbase, size_t basesz, size_t coff, size_t rootoff) {
    unsigned order = ((struct scc_btmap_base *)sbase)->btm_order;
    if (order < 3u) {
        return 0;
    }
    return scc_btmap_impl_new_dyn(sbase, basesz, coff, rootoff);
}

inline size_t scc_btmap_impl_npad(void const *btmap) {
    return ((unsigned char const *)btmap)[-1] + sizeof(unsigned char);
}

#define scc_btmap_impl_base_qual(btmap, qual)                                                       \
    scc_container_qual(                                                                             \
        (unsigned char qual *)(btmap) - scc_btmap_impl_npad(btmap),                                 \
        struct scc_btmap_base,                                                                      \
        btm_fwoff,                                                                                  \
        qual                                                                                        \
    )

#define scc_btmap_impl_base(btmap)                                                                  \
    scc_btmap_impl_base_qual(btmap,)

void scc_btmap_free(void *btmap);

inline size_t scc_btmap_order(void const *btmap) {
    struct scc_btmap_base const *base = scc_btmap_impl_base_qual(btmap, const);
    return base->btm_order;
}

inline size_t scc_btmap_size(void const *btmap) {
    struct scc_btmap_base const *base = scc_btmap_impl_base_qual(btmap, const);
    return base->btm_size;
}

_Bool scc_btmap_impl_insert(void *btmapaddr);

#define scc_btmap_insert(btmapaddr, key, value)                                                         \
    scc_btmap_impl_insert((                                                                             \
            (*(btmapaddr))->btm_key = (key),                                                            \
            (*(btmapaddr))->btm_value = (value),                                                        \
            (btmapaddr)                                                                                 \
        )                                                                                               \
    )

void *scc_btmap_impl_find(void *btmap);

#define scc_btmap_find(btmap, key)                                                                      \
    scc_btmap_impl_find((                                                                               \
            ((btmap)->btm_key = (key)), (btmap)                                                         \
        )                                                                                               \
    )

_Bool scc_btmap_impl_remove(void *btmap);

#define scc_btmap_remove(btmap, key)                                                                    \
    scc_btmap_impl_remove((                                                                             \
            ((btmap)->btm_key = (key)), (btmap)                                                         \
        )                                                                                               \
    )

void *scc_btmap_clone(void const *btmap);

#endif /* SCC_BTMAP_H */
