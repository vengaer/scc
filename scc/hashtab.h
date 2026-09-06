#ifndef SCC_HASHTAB_H
#define SCC_HASHTAB_H

#include <scc/arch.h>
#include <scc/bits.h>
#include <scc/bug.h>
#include <scc/canary.h>
#include <scc/config.h>
#include <scc/hash.h>
#include <scc/mem.h>

#include <stddef.h>

#define scc_hashtab(type) type *

#define scc_hashtab_iter(type) type const *

#define SCC_HASHTAB_GUARDSZ ((unsigned)SCC_VECSIZE - 1u)

#define SCC_HASHTAB_CANARYSZ 32u

#ifndef SCC_HASHTAB_STACKCAP
#define SCC_HASHTAB_STACKCAP 32
#endif

#if SCC_HASHTAB_STACKCAP < 32
#error Stack capacity must be at least 32
#endif

#if !scc_bits_is_power_of_2(SCC_HASHTAB_STACKCAP)
#error Stack capacity must be a power of 2
#endif

typedef _Bool(*scc_hashtab_eq)(void const *, void const *);

typedef scc_hash_type(*scc_hashtab_hash)(void const*, size_t);

typedef unsigned char scc_hashtab_metatype;

struct scc_hashtab_perfevts {
    size_t ev_n_rehashes;
    size_t ev_n_eqs;
    size_t ev_n_hash;
    size_t ev_n_inserts;
    size_t ev_bytesz;
};

struct scc_hashtab_base {
    scc_hashtab_eq ht_eq;
    scc_hashtab_hash ht_hash;
    size_t ht_mdoff;
    size_t ht_size;
    size_t ht_capacity;
#ifdef SCC_PERFEVTS
    struct scc_hashtab_perfevts ht_perf;
#endif
    unsigned char ht_dynalloc;
    unsigned char ht_fwoff;
    unsigned char ht_buffer[];
};

#ifdef SCC_PERFEVTS
#define SCC_HASHTAB_INJECT_PERFEVTS(name)                                   \
    struct scc_hashtab_perfevts name;
#else
#define SCC_HASHTAB_INJECT_PERFEVTS(name)
#endif

#define scc_hashtab_impl_layout(type)                                       \
    struct {                                                                \
        struct {                                                            \
            struct {                                                        \
                scc_hashtab_eq ht_eq;                                       \
                scc_hashtab_hash ht_hash;                                   \
                size_t ht_mdoff;                                            \
                size_t ht_size;                                             \
                size_t ht_capacity;                                         \
                SCC_HASHTAB_INJECT_PERFEVTS(ht_perf)                        \
                unsigned char ht_dynalloc;                                  \
                unsigned char ht_fwoff;                                     \
                unsigned char ht_bkoff;                                     \
            } ht0;                                                          \
            type ht_curr;                                                   \
            type ht_data[SCC_HASHTAB_STACKCAP];                             \
        } ht1;                                                              \
        scc_hashtab_metatype ht_meta[SCC_HASHTAB_STACKCAP];                 \
        scc_hashtab_metatype ht_guard[SCC_HASHTAB_GUARDSZ];                 \
        SCC_CANARY_INJECT(SCC_HASHTAB_CANARYSZ)                             \
    }

#define scc_hashtab_impl_curroff(type)                                      \
    sizeof(                                                                 \
        struct {                                                            \
            struct {                                                        \
                scc_hashtab_eq ht_eq;                                       \
                scc_hashtab_hash ht_hash;                                   \
                size_t ht_mdoff;                                            \
                size_t ht_size;                                             \
                size_t ht_capacity;                                         \
                SCC_HASHTAB_INJECT_PERFEVTS(ht_perf)                        \
                unsigned char ht_dynalloc;                                  \
                unsigned char ht_fwoff;                                     \
                unsigned char ht_bkoff;                                     \
            } ht0;                                                          \
            type ht_curr[];                                                 \
        }                                                                   \
    )

#define scc_hashtab_impl_metaoff(type)                                      \
    sizeof(                                                                 \
        struct {                                                            \
            struct {                                                        \
                struct {                                                    \
                    scc_hashtab_eq ht_eq;                                   \
                    scc_hashtab_hash ht_hash;                               \
                    size_t ht_mdoff;                                        \
                    size_t ht_size;                                         \
                    size_t ht_capacity;                                     \
                    SCC_HASHTAB_INJECT_PERFEVTS(ht_perf)                    \
                    unsigned char ht_dynalloc;                              \
                    unsigned char ht_fwoff;                                 \
                    unsigned char ht_bkoff;                                 \
                } ht0;                                                      \
                type ht_curr;                                               \
                type ht_data[SCC_HASHTAB_STACKCAP];                         \
            } ht1;                                                          \
            scc_hashtab_metatype ht_meta[];                                 \
        }                                                                   \
    )

void *scc_hashtab_impl_new(struct scc_hashtab_base *base, size_t coff, size_t mdoff);

void *scc_hashtab_impl_new_dyn(scc_hashtab_eq eq, scc_hashtab_hash hash, size_t cap, size_t tabsz, size_t coff, size_t mdoff);

#define scc_hashtab_with_hash(type, eq, hash)                               \
    (type *)scc_hashtab_impl_new(                                           \
        (void *)&(scc_hashtab_impl_layout(type)) {                          \
            .ht1 = {                                                        \
                .ht0 = {                                                    \
                    .ht_eq = eq,                                            \
                    .ht_hash = hash,                                        \
                    .ht_capacity = SCC_HASHTAB_STACKCAP                     \
                },                                                          \
            },                                                              \
        },                                                                  \
        scc_hashtab_impl_curroff(type),                                     \
        scc_hashtab_impl_metaoff(type)                                      \
    )

#define scc_hashtab_with_hash_dyn(type, eq, hash)                           \
    (type *)scc_hashtab_impl_new_dyn(                                       \
        eq,                                                                 \
        hash,                                                               \
        SCC_HASHTAB_STACKCAP,                                               \
        sizeof(scc_hashtab_impl_layout(type)),                              \
        scc_hashtab_impl_curroff(type),                                     \
        scc_hashtab_impl_metaoff(type)                                      \
    )

#define scc_hashtab_new(type, eq)                                           \
    scc_hashtab_with_hash(type, eq, scc_hash_fnv1a)

#define scc_hashtab_new_dyn(type, eq)                                       \
    scc_hashtab_with_hash_dyn(type, eq, scc_hash_fnv1a)

inline size_t scc_hashtab_impl_bkpad(void const *tab) {
    return ((unsigned char const *)tab)[-1] + sizeof(((struct scc_hashtab_base *)0)->ht_fwoff);
}

#define scc_hashtab_impl_base_qual(tab, qual)                               \
    scc_container_qual(                                                     \
        (unsigned char qual *)(tab) - scc_hashtab_impl_bkpad(tab),          \
        struct scc_hashtab_base,                                            \
        ht_fwoff,                                                           \
        qual                                                                \
    )

#define scc_hashtab_impl_base(tab)                                          \
    scc_hashtab_impl_base_qual(tab,)

void scc_hashtab_free(void *tab);

_Bool scc_hashtab_impl_insert(void *tabaddr, size_t elemsize);

#define scc_hashtab_insert(tabaddr, value)                                  \
    scc_hashtab_impl_insert((**(tabaddr) = (value), (tabaddr)), sizeof(**(tabaddr)))

inline size_t scc_hashtab_capacity(void const *tab) {
    struct scc_hashtab_base const *base = scc_hashtab_impl_base_qual(tab, const);
    return base->ht_capacity;
}

inline size_t scc_hashtab_size(void const *tab) {
    struct scc_hashtab_base const *base = scc_hashtab_impl_base_qual(tab, const);
    return base->ht_size;
}

void const *scc_hashtab_impl_find(void const *tab, size_t elemsize);

#define scc_hashtab_find(tab, value)                                        \
    scc_hashtab_impl_find((*(tab) = (value), (tab)), sizeof(*(tab)))

_Bool scc_hashtab_impl_reserve(void *tabaddr, size_t capacity, size_t elemsize);

#define scc_hashtab_reserve(tabaddr, capacity)                              \
    scc_hashtab_impl_reserve(tabaddr, capacity, sizeof(**(tabaddr)))

_Bool scc_hashtab_impl_remove(void *tab, size_t elemsize);

#define scc_hashtab_remove(tab, value)                                      \
    scc_hashtab_impl_remove((*(tab) = (value), (tab)), sizeof(*(tab)))

void scc_hashtab_clear(void *tab);

void *scc_hashtab_clone(void const *tab);

#define scc_hashtab_foreach(iter, tab)                                      \
    for ((iter) = scc_hashtab_impl_iter_begin(tab, sizeof(*(tab)));         \
         (iter);                                                            \
         (iter) = scc_hashtab_impl_iter_next(tab, sizeof(*(tab)), iter))

void const *scc_hashtab_impl_iter_begin(void *tab, size_t elemsize);

void const *scc_hashtab_impl_iter_next(void *tab, size_t elemsize, void const *iter);

#endif /* SCC_HASHTAB_H */
