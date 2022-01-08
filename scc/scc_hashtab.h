#ifndef SCC_HASHTAB_H
#define SCC_HASHTAB_H

#include "scc_mem.h"

#include <stddef.h>

enum { SCC_HASHTAB_STACKCAP = 32 };

typedef _Bool(*scc_eq)(void const *, void const *);
typedef unsigned long long(*scc_hash)(void const *, size_t);

#define scc_hashtab(type) type *

struct scc_hashtab {
    scc_eq ht_eq;
    scc_hash ht_hash;
    size_t ht_mdoff;
    size_t ht_size;
    size_t ht_capacity;
    unsigned char ht_dynalloc;
    unsigned char ht_fwoff;
    unsigned char ht_buffer[];
};

typedef unsigned short scc_hashtab_metatype;

#define scc_hashtab_impl_layout(type)                                   \
    struct {                                                            \
        scc_eq ht_eq;                                                   \
        scc_hash ht_hash;                                               \
        size_t ht_mdoff;                                                \
        size_t ht_size;                                                 \
        size_t ht_capacity;                                             \
        unsigned char ht_dynalloc;                                      \
        unsigned char ht_fwoff;                                         \
        unsigned char ht_bkoff;                                         \
        type ht_tmp;                                                    \
        type ht_data[SCC_HASHTAB_STACKCAP];                             \
        scc_hashtab_metatype ht_meta[SCC_HASHTAB_STACKCAP];             \
        scc_hashtab_metatype ht_meta_guard[scc_hashtab_impl_guardsz()]; \
    }

#define scc_hashtab_impl_initsize(type)                                 \
    sizeof(scc_hashtab_impl_layout(type))

#define scc_hashtab_impl_inittab(type)                                  \
    (union {                                                            \
        struct scc_hashtab sc_tab;                                      \
        unsigned char sc_buffer[scc_hashtab_impl_initsize(type)];       \
    }){ 0 }.sc_tab

#define scc_hashtab_impl_guardsz()                                      \
    (SCC_VECSIZE / sizeof(scc_hashtab_metatype) - 1u)

#define scc_hashtab_impl_dataoff(type)                                  \
    offsetof(scc_hashtab_impl_layout(type), ht_tmp)

#define scc_hashtab_impl_mdoff(type)                                    \
    offsetof(scc_hashtab_impl_layout(type), ht_meta)

#define scc_hashtab_impl_base_qual(tab, qual)                           \
    scc_container_qual(                                                 \
        tab - scc_hashtab_impl_bkoff(tab),                              \
        struct scc_hashtab,                                             \
        ht_fwoff,                                                       \
        qual                                                            \
    )

#define scc_hashtab_impl_base(tab)                                      \
    scc_hashtab_impl_base_qual(tab,)

#define scc_hashtab_init(type, eq)                                      \
    scc_hashtab_with_hash(type, eq, scc_hashtab_fnv1a)

#define scc_hashtab_with_hash(type, eq, hash)                           \
    scc_hashtab_impl_init(                                              \
        &scc_hashtab_impl_inittab(type),                                \
        eq,                                                             \
        hash,                                                           \
        scc_hashtab_impl_dataoff(type),                                 \
        scc_hashtab_impl_mdoff(type),                                   \
        SCC_HASHTAB_STACKCAP                                            \
    )

void *scc_hashtab_impl_init(void *inittab, scc_eq eq, scc_hash hash, size_t dataoff, size_t mdoff, size_t capacity);

void scc_hashtab_free(void *tab);
unsigned long long scc_hashtab_fnv1a(void const *input, size_t size);

inline size_t scc_hashtab_impl_bkoff(void const *tab) {
    return ((unsigned char const *)tab)[-1] + sizeof(((struct scc_hashtab *)0)->ht_fwoff);
}

inline size_t scc_hashtab_capacity(void const *tab) {
    return scc_hashtab_impl_base_qual(tab, const)->ht_capacity;
}

inline size_t scc_hashtab_size(void const *tab) {
    return scc_hashtab_impl_base_qual(tab, const)->ht_size;
}

_Bool scc_hashtab_impl_insert(void *tab, size_t elemsize);

#define scc_hashtab_insert(tab, value)                                  \
    scc_hashtab_impl_insert((*(tab) = (value), &(tab)), sizeof(*tab))

void const *scc_hashtab_impl_find(void const *tab, size_t elemsize);

#define scc_hashtab_find(tab, value)                                    \
    scc_hashtab_impl_find((*(tab) = (value), (tab)), sizeof(value))

_Bool scc_hashtab_impl_remove(void *tab, size_t elemsize);

#define scc_hashtab_remove(tab, value)                                  \
    scc_hashtab_impl_remove((*(tab) = (value), (tab)), sizeof(value))

#endif /* SCC_HASHTAB_H */
