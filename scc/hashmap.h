#ifndef SCC_HASHMAP_H
#define SCC_HASHMAP_H

#include "arch.h"
#include "bits.h"
#include "bug.h"
#include "canary.h"
#include "config.h"
#include "hash.h"
#include "mem.h"
#include "perf.h"

#include <stddef.h>

#define scc_hashmap_impl_pair(keytype, valuetype)                                       \
    struct { keytype hp_key; valuetype hp_val; }

#define scc_hashmap_impl_pair_valoff(keytype, valuetype)                                \
    scc_align(sizeof(keytype), scc_alignof(valuetype))

#define scc_hashmap(keytype, valuetype)                                                 \
    scc_hashmap_impl_pair(keytype, valuetype) *

#define SCC_HASHMAP_GUARDSZ ((unsigned)SCC_VECSIZE - 1u)

#define SCC_HASHMAP_CANARYSZ 32u

#define SCC_HASHMAP_DUPLICATE (~(~0ull >> 1u))

#ifndef SCC_HASHMAP_STACKCAP

#define SCC_HASHMAP_STACKCAP 32
#endif

#if SCC_HASHMAP_STACKCAP < 32
#error Stack capacity must be at least 32
#endif

#if !scc_bits_is_power_of_2(SCC_HASHMAP_STACKCAP)
#error Stack capacity must be a power of 2
#endif

typedef _Bool(*scc_hashmap_eq)(void const *, void const *);

typedef scc_hash_type(*scc_hashmap_hash)(void const *, size_t);

typedef unsigned char scc_hashmap_metatype;

struct scc_hashmap_perfevts {
    size_t ev_n_rehashes;
    size_t ev_n_eqs;
    size_t ev_n_hash;
    size_t ev_n_inserts;
    size_t ev_bytesz;
};

struct scc_hashmap_base {
    scc_hashmap_eq hm_eq;
    scc_hashmap_hash hm_hash;
    size_t hm_valoff;
    size_t hm_mdoff;
    size_t hm_size;
    size_t hm_capacity;
    size_t hm_pairsize;
#ifdef SCC_PERFEVTS
    struct scc_hashmap_perfevts hm_perf;
#endif
    unsigned short hm_keyalign;
    unsigned short hm_valalign;
    unsigned char hm_dynalloc;
    unsigned char hm_valpad;
    unsigned char hm_fwoff;
    unsigned char hm_buffer[];
};

#ifdef SCC_PERFEVTS
#define SCC_HASHMAP_INJECT_PERFEVTS(name)                                   \
    struct scc_hashmap_perfevts name;
#else
#define SCC_HASHMAP_INJECT_PERFEVTS(name)
#endif

#define scc_hashmap_impl_layout(keytype, valuetype)                                         \
    struct {                                                                                \
        struct {                                                                            \
            struct {                                                                        \
                struct {                                                                    \
                    scc_hashmap_eq hm_eq;                                                   \
                    scc_hashmap_hash hm_hash;                                               \
                    size_t hm_valoff;                                                       \
                    size_t hm_mdoff;                                                        \
                    size_t hm_size;                                                         \
                    size_t hm_capacity;                                                     \
                    size_t hm_pairsize;                                                     \
                    SCC_HASHMAP_INJECT_PERFEVTS(hm_perf)                                    \
                    unsigned short hm_keyalign;                                             \
                    unsigned short hm_valalign;                                             \
                    unsigned char hm_dynalloc;                                              \
                    unsigned char hm_valpad;                                                \
                    unsigned char hm_fwoff;                                                 \
                    unsigned char hm_bkoff;                                                 \
                } hm0;                                                                      \
                scc_hashmap_impl_pair(keytype, valuetype) hm_curr;                          \
                keytype hm_keys[SCC_HASHMAP_STACKCAP];                                      \
            } hm1;                                                                          \
            valuetype hm_vals[SCC_HASHMAP_STACKCAP];                                        \
        } hm2;                                                                              \
        scc_hashmap_metatype hm_meta[SCC_HASHMAP_STACKCAP];                                 \
        scc_hashmap_metatype hm_guard[SCC_HASHMAP_GUARDSZ];                                 \
        SCC_CANARY_INJECT(SCC_HASHMAP_CANARYSZ)                                             \
    }

#define scc_hashmap_impl_curroff(keytype, valuetype)                                        \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                scc_hashmap_eq hm_eq;                                                       \
                scc_hashmap_hash hm_hash;                                                   \
                size_t hm_valoff;                                                           \
                size_t hm_mdoff;                                                            \
                size_t hm_size;                                                             \
                size_t hm_capacity;                                                         \
                size_t hm_pairsize;                                                         \
                SCC_HASHMAP_INJECT_PERFEVTS(hm_perf)                                        \
                unsigned short hm_keyalign;                                                 \
                unsigned short hm_valalign;                                                 \
                unsigned char hm_dynalloc;                                                  \
                unsigned char hm_valpad;                                                    \
                unsigned char hm_fwoff;                                                     \
                unsigned char hm_bkoff;                                                     \
            } hm0;                                                                          \
            scc_hashmap_impl_pair(keytype, valuetype) hm_curr[];                            \
        }                                                                                   \
    )

#define scc_hashmap_impl_valoff(keytype, valuetype)                                         \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                struct {                                                                    \
                    scc_hashmap_eq hm_eq;                                                   \
                    scc_hashmap_hash hm_hash;                                               \
                    size_t hm_valoff;                                                       \
                    size_t hm_mdoff;                                                        \
                    size_t hm_size;                                                         \
                    size_t hm_capacity;                                                     \
                    size_t hm_pairsize;                                                     \
                    SCC_HASHMAP_INJECT_PERFEVTS(hm_perf)                                    \
                    unsigned short hm_keyalign;                                             \
                    unsigned short hm_valalign;                                             \
                    unsigned char hm_dynalloc;                                              \
                    unsigned char hm_valpad;                                                \
                    unsigned char hm_fwoff;                                                 \
                    unsigned char hm_bkoff;                                                 \
                } hm0;                                                                      \
                scc_hashmap_impl_pair(keytype, valuetype) hm_curr;                          \
                keytype hm_keys[SCC_HASHMAP_STACKCAP];                                      \
            } hm1;                                                                          \
            valuetype hm_vals[];                                                            \
        }                                                                                   \
    )

#define scc_hashmap_impl_mdoff(keytype, valuetype)                                          \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                struct {                                                                    \
                    struct {                                                                \
                        scc_hashmap_eq hm_eq;                                               \
                        scc_hashmap_hash hm_hash;                                           \
                        size_t hm_valoff;                                                   \
                        size_t hm_mdoff;                                                    \
                        size_t hm_size;                                                     \
                        size_t hm_capacity;                                                 \
                        size_t hm_pairsize;                                                 \
                        SCC_HASHMAP_INJECT_PERFEVTS(hm_perf)                                \
                        unsigned short hm_keyalign;                                         \
                        unsigned short hm_valalign;                                         \
                        unsigned char hm_dynalloc;                                          \
                        unsigned char hm_valpad;                                            \
                        unsigned char hm_fwoff;                                             \
                        unsigned char hm_bkoff;                                             \
                    } hm0;                                                                  \
                    scc_hashmap_impl_pair(keytype, valuetype) hm_curr;                      \
                    keytype hm_keys[SCC_HASHMAP_STACKCAP];                                  \
                } hm1;                                                                      \
                valuetype hm_vals[SCC_HASHMAP_STACKCAP];                                    \
            } hm2;                                                                          \
            scc_hashmap_metatype hm_meta[];                                                 \
        }                                                                                   \
    )

void *scc_hashmap_impl_new(struct scc_hashmap_base *base, size_t coff, size_t valoff, size_t keysize);

void *scc_hashmap_impl_new_dyn(struct scc_hashmap_base const *sbase, size_t mapsize, size_t coff, size_t valoff, size_t keysize);

#define scc_hashmap_with_hash(keytype, valuetype, eq, hash)                                 \
    scc_hashmap_impl_new(                                                                   \
        (void *)&(scc_hashmap_impl_layout(keytype, valuetype)){                             \
            .hm2 = {                                                                        \
                .hm1 = {                                                                    \
                    .hm0 = {                                                                \
                        .hm_eq = eq,                                                        \
                        .hm_hash = hash,                                                    \
                        .hm_valoff = scc_hashmap_impl_valoff(keytype, valuetype),           \
                        .hm_mdoff = scc_hashmap_impl_mdoff(keytype, valuetype),             \
                        .hm_capacity = SCC_HASHMAP_STACKCAP,                                \
                        .hm_pairsize = sizeof(scc_hashmap_impl_pair(keytype, valuetype)),   \
                        .hm_keyalign = scc_alignof(keytype),                                \
                        .hm_valalign = scc_alignof(valuetype)                               \
                    },                                                                      \
                },                                                                          \
            },                                                                              \
        },                                                                                  \
        scc_hashmap_impl_curroff(keytype, valuetype),                                       \
        scc_hashmap_impl_pair_valoff(keytype, valuetype),                                   \
        sizeof(keytype)                                                                     \
    )

#define scc_hashmap_with_hash_dyn(keytype, valuetype, eq, hash)                             \
    scc_hashmap_impl_new_dyn(                                                               \
        (void *)&(struct scc_hashmap_base){                                                 \
            .hm_eq = eq,                                                                    \
            .hm_hash = hash,                                                                \
            .hm_valoff = scc_hashmap_impl_valoff(keytype, valuetype),                       \
            .hm_mdoff = scc_hashmap_impl_mdoff(keytype, valuetype),                         \
            .hm_capacity = SCC_HASHMAP_STACKCAP,                                            \
            .hm_pairsize = sizeof(scc_hashmap_impl_pair(keytype, valuetype)),               \
            .hm_keyalign = scc_alignof(keytype),                                            \
            .hm_valalign = scc_alignof(valuetype)                                           \
        },                                                                                  \
        sizeof(scc_hashmap_impl_layout(keytype, valuetype)),                                \
        scc_hashmap_impl_curroff(keytype, valuetype),                                       \
        scc_hashmap_impl_pair_valoff(keytype, valuetype),                                   \
        sizeof(keytype)                                                                     \
    )

#define scc_hashmap_new(keytype, valuetype, eq)                                            \
    scc_hashmap_with_hash(keytype, valuetype, eq, scc_hash_fnv1a)

#define scc_hashmap_new_dyn(keytype, valuetype, eq)                                       \
    scc_hashmap_with_hash_dyn(keytype, valuetype, eq, scc_hash_fnv1a)

inline size_t scc_hashmap_impl_bkpad(void const *map) {
    return ((unsigned char const *)map)[-1] + sizeof(((struct scc_hashmap_base *)0)->hm_fwoff);
}

#define scc_hashmap_impl_base_qual(map, qual)                               \
    scc_container_qual(                                                     \
        (unsigned char qual *)(map) - scc_hashmap_impl_bkpad(map),          \
        struct scc_hashmap_base,                                            \
        hm_fwoff,                                                           \
        qual                                                                \
    )

#define scc_hashmap_impl_base(map)                                          \
    scc_hashmap_impl_base_qual(map,)

void scc_hashmap_free(void *map);

_Bool scc_hashmap_impl_insert(void *mapaddr, size_t keysize, size_t valsize);

#define scc_hashmap_insert(mapaddr, key, value)                             \
    scc_hashmap_impl_insert((                                               \
            (*(mapaddr))->hp_key = (key),                                   \
            (*(mapaddr))->hp_val = (value),                                 \
            (mapaddr)                                                       \
        ),                                                                  \
        sizeof((*(mapaddr))->hp_key),                                       \
        sizeof((*(mapaddr))->hp_val)                                        \
    )

inline size_t scc_hashmap_capacity(void const *map) {
    struct scc_hashmap_base const *base =
        scc_hashmap_impl_base_qual(map, const);
    return base->hm_capacity;
}

inline size_t scc_hashmap_size(void const *map) {
    struct scc_hashmap_base const *base =
        scc_hashmap_impl_base_qual(map, const);
    return base->hm_size;
}

void *scc_hashmap_impl_find(void *map, size_t keysize, size_t valsize);

#define scc_hashmap_find(map, key)                                      \
    scc_hashmap_impl_find(                                              \
        ((map)->hp_key = (key), (map)),                                 \
        sizeof((map)->hp_key),                                          \
        sizeof((map)->hp_val)                                           \
    )

_Bool scc_hashmap_impl_remove(void *map, size_t keysize);

#define scc_hashmap_remove(map, key)                                    \
    scc_hashmap_impl_remove(((map)->hp_key = (key), (map)), sizeof((map)->hp_key))

void scc_hashmap_clear(void *map);

void *scc_hashmap_clone(void const *map);

#endif /* SCC_HASHMAP_H */
