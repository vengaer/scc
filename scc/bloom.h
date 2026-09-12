/*
 * Copyright (c) 2026 Vilhelm Engström
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SCC_BLOOM_H
#define SCC_BLOOM_H

#include <scc/config.h>
#include <scc/hash.h>
#include <scc/mem.h>

#include <stddef.h>
#include <stdint.h>

#if defined SCC_HAVE_UINT32_T || defined SCC_HAVE_UINT64_T

#define scc_bloom(type) type *

typedef void (*scc_bloom_hash)(struct scc_digest128 *, void const *, size_t, uint_fast32_t);

struct scc_bloom_base {
    scc_bloom_hash bm_hash;
    unsigned bm_nbits;
    unsigned bm_nhashes;
    unsigned char bm_tail[];
};

#define scc_bloom_impl_layout(type, m)                                                             \
    struct {                                                                                       \
        struct {                                                                                   \
            scc_bloom_hash bm_hash;                                                                \
            unsigned bm_nbits;                                                                     \
            unsigned bm_nhashes;                                                                   \
            unsigned char bm_npad;                                                                 \
            unsigned char bm_dynalloc;                                                             \
        } bm_base;                                                                                 \
        type bm_tmp;                                                                               \
        unsigned char bm_buckets[((m + 7u) & ~7u) >> 3u];                                          \
    }

#define scc_bloom_impl_offset(type)                                                                \
    sizeof(struct {                                                                                \
        struct {                                                                                   \
            scc_bloom_hash bm_hash;                                                                \
            unsigned bm_nbits;                                                                     \
            unsigned bm_nhashes;                                                                   \
            unsigned char bm_npad;                                                                 \
            unsigned char bm_dynalloc;                                                             \
        } bm_base;                                                                                 \
        type bm_tmp[];                                                                             \
    })

#define scc_bloom_new(type, m, k)                                                                  \
    (type *)scc_bloom_impl_new((void *)&(scc_bloom_impl_layout(type, m)){0},                       \
                               scc_bloom_impl_offset(type), (m), (k))

void *scc_bloom_impl_new(struct scc_bloom_base *base, size_t offset, unsigned m, unsigned k);

#define scc_bloom_with_hash(type, m, k, hash)                                                      \
    (type *)scc_bloom_impl_with_hash((void *)&(scc_bloom_impl_layout(type, m)){0},                 \
                                     scc_bloom_impl_offset(type), (m), (k), (hash))

void *scc_bloom_impl_with_hash(struct scc_bloom_base *base, size_t offset, unsigned m, unsigned k,
                               scc_bloom_hash hash);

#define scc_bloom_new_dyn(type, m, k)                                                              \
    (type *)scc_bloom_impl_new_dyn(sizeof(scc_bloom_impl_layout(type, m)),                         \
                                   scc_bloom_impl_offset(type), (m), (k))

void *scc_bloom_impl_new_dyn(size_t size, size_t offset, unsigned m, unsigned k);

#define scc_bloom_with_hash_dyn(type, m, k, hash)                                                  \
    (type *)scc_bloom_impl_with_hash_dyn(sizeof(scc_bloom_impl_layout(type, m)),                   \
                                         scc_bloom_impl_offset(type), (m), (k), (hash))

void *scc_bloom_impl_with_hash_dyn(size_t size, size_t offset, unsigned m, unsigned k,
                                   scc_bloom_hash hash);

inline size_t scc_bloom_impl_npad(void const *flt)
{
    return ((unsigned char const *)flt)[-2] + (sizeof(unsigned char) << 1u);
}

#define scc_bloom_impl_base_qual(flt, qual)                                                        \
    scc_container_qual((unsigned char qual *)(flt) - scc_bloom_impl_npad(flt),                     \
                       struct scc_bloom_base, bm_tail, qual)

#define scc_bloom_impl_base(flt) scc_bloom_impl_base_qual(flt, )

void scc_bloom_free(void *flt);

#define scc_bloom_insert(fltaddr, value)                                                           \
    scc_bloom_impl_insert(((void)(**(fltaddr) = value), *(fltaddr)), sizeof(**(fltaddr)))

void scc_bloom_impl_insert(void *flt, size_t elemsize);

#define scc_bloom_test(flt, value)                                                                 \
    scc_bloom_impl_test(((void)(*(flt) = value), (flt)), sizeof(*(flt)))

_Bool scc_bloom_impl_test(void *flt, size_t elemsize);

inline size_t scc_bloom_capacity(void const *flt)
{
    struct scc_bloom_base const *base = scc_bloom_impl_base_qual(flt, const);
    return base->bm_nbits >> 3u;
}

inline size_t scc_bloom_nhashes(void const *flt)
{
    struct scc_bloom_base const *base = scc_bloom_impl_base_qual(flt, const);
    return base->bm_nhashes;
}

#ifdef SCC_HAVE_LIBM
#define scc_bloom_size(flt) scc_bloom_impl_size(flt, sizeof(*flt))

size_t scc_bloom_impl_size(void const *flt, size_t elemsize);
#endif /* SCC_HAVE_LIBM */

#define scc_bloom_clone(flt) scc_bloom_impl_clone(flt, sizeof(*(flt)))

void *scc_bloom_impl_clone(void const *flt, size_t elemsize);

#endif /* SCC_HAVE_UINT32_T || SCC_HAVE_UINT64_T */

#endif /* SCC_BLOOM_H */
