#ifndef SCC_VEC_H
#define SCC_VEC_H

#include "bug.h"
#include "mem.h"
#include "pp_token.h"

#include <stddef.h>

#ifndef SCC_VEC_STATIC_CAPACITY
#define SCC_VEC_STATIC_CAPACITY 32
#endif

#if SCC_VEC_STATIC_CAPACITY <= 0
#error Static capacity must be greater than 0
#endif

#define scc_vec(type) type *

#define scc_vec_iter(type) scc_vec(type)

struct scc_vec_base {
    size_t sv_size;
    size_t sv_capacity;
    unsigned char sv_buffer[];
};

#define scc_vec_impl_layout(type)                                       \
    struct {                                                            \
        struct {                                                        \
            size_t sv_size;                                             \
            size_t sv_capacity;                                         \
            unsigned char sv_npad;                                      \
            unsigned char sv_dynalloc;                                  \
        } v0;                                                           \
        type sv_buffer[SCC_VEC_STATIC_CAPACITY];                        \
    }

#define scc_vec_impl_base_qual(vec, qual)                               \
    scc_container_qual(                                                 \
        (unsigned char qual *)(vec) - scc_vec_impl_npad(vec),           \
        struct scc_vec_base,                                            \
        sv_buffer,                                                      \
        qual                                                            \
    )

#define scc_vec_impl_base(vec)                                          \
    scc_vec_impl_base_qual(vec,)

#define scc_vec_impl_offset(type)                                       \
    sizeof(                                                             \
        struct {                                                        \
            struct {                                                    \
                size_t sv_size;                                         \
                size_t sv_capacity;                                     \
                unsigned char sv_npad;                                  \
                unsigned char sv_dynalloc;                              \
            } v0;                                                       \
            type sv_buffer[];                                           \
        }                                                               \
    )

void *scc_vec_impl_new(struct scc_vec_base *base, size_t offset, size_t capacity);

void *scc_vec_impl_new_dyn(size_t vecsz, size_t offset, size_t capacity);

#define scc_vec_new(type)                                                \
    (type *)scc_vec_impl_new(                                            \
        (void *)&(scc_vec_impl_layout(type)){ 0 },                       \
        scc_vec_impl_offset(type),                                       \
        SCC_VEC_STATIC_CAPACITY                                          \
    )

#define scc_vec_new_dyn(type)                                            \
    (type *)scc_vec_impl_new_dyn(                                        \
        sizeof(scc_vec_impl_layout(type)),                               \
        scc_vec_impl_offset(type),                                       \
        SCC_VEC_STATIC_CAPACITY                                          \
    )

void *scc_vec_impl_from(void *restrict vec, void const *restrict data, size_t size, size_t elemsize);

void *scc_vec_impl_from_dyn(size_t basecap, size_t offset, void const *data, size_t size, size_t elemsize);

#define scc_vec_from(type, ...)                                             \
    scc_vec_impl_from(                                                      \
        scc_vec_new(type),                                                  \
        (type[]){ __VA_ARGS__ },                                            \
        scc_arrsize(((type[]){ __VA_ARGS__ })),                             \
        sizeof(type)                                                        \
    )

#define scc_vec_from_dyn(type, ...)                                         \
    scc_vec_impl_from_dyn(                                                  \
        SCC_VEC_STATIC_CAPACITY,                                            \
        scc_vec_impl_offset(type),                                          \
        (type[]){ __VA_ARGS__ },                                            \
        scc_arrsize(((type[]){ __VA_ARGS__ })),                             \
        sizeof(type)                                                        \
    )

_Bool scc_vec_impl_resize(void *vecaddr, size_t size, size_t elemsize);

#define scc_vec_resize(vecaddr, size)                                       \
    scc_vec_impl_resize(vecaddr, size, sizeof(**(vecaddr)))

void scc_vec_impl_erase(void *vec, size_t index, size_t elemsize);

#define scc_vec_erase(vec, index)                                       \
    scc_vec_impl_erase(vec, index, sizeof(*(vec)))

void scc_vec_impl_erase_range(void *vec, size_t first, size_t end, size_t elemsize);

#define scc_vec_erase_range(vec, first, end)                            \
    scc_vec_impl_erase_range(vec, first, end, sizeof(*(vec)))

_Bool scc_vec_impl_push_ensure_capacity(void *vecaddr, size_t elemsize);

#define scc_vec_push(vecaddr, value)                                        \
    (scc_vec_impl_push_ensure_capacity(vecaddr, sizeof(**(vecaddr))) &&     \
    ((*(vecaddr))[scc_vec_impl_base(*(vecaddr))->sv_size++] = (value),1))

_Bool scc_vec_impl_reserve(void *vecaddr, size_t capacity, size_t elemsize);

#define scc_vec_reserve(vecaddr, capacity)                                  \
    scc_vec_impl_reserve((vecaddr), capacity, sizeof(**(vecaddr)))

inline size_t scc_vec_impl_npad(void const *vec) {
    return ((unsigned char const *)vec)[-2] + 2 * sizeof(unsigned char);
}

void scc_vec_free(void *vec);

inline size_t scc_vec_size(void const *vec) {
    return scc_vec_impl_base_qual(vec, const)->sv_size;
}

inline size_t scc_vec_capacity(void const *vec) {
    return scc_vec_impl_base_qual(vec, const)->sv_capacity;
}

inline _Bool scc_vec_empty(void const *vec) {
    return !scc_vec_size(vec);
}

#define scc_vec_pop(vec)                                                    \
    ((vec)[--scc_vec_impl_base(vec)->sv_size])

inline void scc_vec_clear(void *vec) {
    scc_vec_impl_base(vec)->sv_size = 0u;
}

inline _Bool scc_vec_is_allocd(void const *vec) {
    return ((unsigned char const*)vec)[-1];
}

void *scc_vec_impl_clone(void const *vec, size_t elemsize);

#define scc_vec_clone(vec)                                              \
    scc_vec_impl_clone(vec, sizeof(*(vec)))

#define scc_vec_foreach(iter, vec)                                      \
    scc_vec_foreach_by(iter, vec, 1)

#define scc_vec_foreach_reversed(iter, vec)                             \
    scc_vec_foreach_reversed_by(iter, vec, 1)

#define scc_vec_foreach_by(iter, vec, by)                               \
    for (void const *scc_pp_cat_expand(scc_vec_end,__LINE__) =           \
            ((iter) = (vec), &(vec)[scc_vec_size(vec)]);                \
        (void const *)(iter) < scc_pp_cat_expand(scc_vec_end,__LINE__); \
        (iter) += (by))

#define scc_vec_foreach_reversed_by(iter, vec, by)                      \
    for ((iter) = &(vec)[scc_vec_size(vec) - 1u]; (iter) >= (vec); (iter) -= (by))

#endif /* SCC_VEC_H */
