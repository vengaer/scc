#ifndef SCC_VEC_H
#define SCC_VEC_H

#include "scc_mem.h"
#include "scc_pp_token.h"

#include <stddef.h>

#define scc_vec(type) type *

struct scc_vec {
    size_t sc_size;
    size_t sc_capacity;
    unsigned char sc_buffer[];
};

#define scc_vec_impl_layout(type)                                   \
    struct {                                                        \
        size_t sc_size;                                             \
        size_t sc_capacity;                                         \
        unsigned char sc_npad;                                      \
        type sc_buffer[];                                           \
    }

#define scc_vec_impl_nullsize(type)                                 \
    offsetof(scc_vec_impl_layout(type), sc_buffer)

#define scc_vec_impl_nullvec(type)                                  \
    (union {                                                        \
        struct scc_vec sc_vec;                                      \
        unsigned char sc_buffer[scc_vec_impl_nullsize(type)];       \
    }){ 0 }.sc_vec

#define scc_vec_impl_base_qual(vec, qual)                           \
    scc_container_qual(vec - scc_vec_impl_npad(vec), struct scc_vec, sc_buffer, qual)

#define scc_vec_impl_base(vec)                                      \
    scc_vec_impl_base_qual(vec,)

void *scc_vec_impl_init(void *nullvec, size_t offset);
void *scc_vec_impl_from(
    size_t offset,
    void const *data,
    size_t size,
    size_t elemsize
);
_Bool scc_vec_impl_resize(void *vec, size_t size, size_t elemsize);
_Bool scc_vec_impl_push_ensure_capacity(void *vec, size_t elemsize);
_Bool scc_vec_impl_reserve(void *vec, size_t capacity, size_t elemsize);
void scc_vec_impl_erase(void *vec, size_t index, size_t elemsize);
void scc_vec_impl_erase_range(void *vec, size_t first, size_t end, size_t elemsize);
size_t scc_vec_impl_at_check(void *vec, size_t index);

inline size_t scc_vec_impl_npad(void const *vec) {
    return ((unsigned char const *)vec)[-1] + sizeof(unsigned char);
}

void scc_vec_free(void *vec);

inline size_t scc_vec_size(void const *vec) {
    return scc_vec_impl_base_qual(vec, const)->sc_size;
}

inline size_t scc_vec_capacity(void const *vec) {
    return scc_vec_impl_base_qual(vec, const)->sc_capacity;
}

inline _Bool scc_vec_empty(void const *vec) {
    return !scc_vec_size(vec);
}

inline void scc_vec_pop(void *vec) {
    --scc_vec_impl_base(vec)->sc_size;
}

inline void scc_vec_clear(void *vec) {
    scc_vec_impl_base(vec)->sc_size = 0u;
}

void scc_vec_pop_safe(void *vec);

#define scc_vec_init(type)                                              \
    scc_vec_impl_init(                                                  \
        &scc_vec_impl_nullvec(type),                                    \
        scc_vec_impl_nullsize(type)                                     \
    )

#define scc_vec_from(type, ...)                                         \
    scc_vec_impl_from(                                                  \
        scc_vec_impl_nullsize(type),                                    \
        (type[]){ __VA_ARGS__ },                                        \
        scc_arrsize(((type[]){ __VA_ARGS__ })),                         \
        sizeof(type)                                                    \
    )

#define scc_vec_reserve(vec, capacity)                                  \
    scc_vec_impl_reserve(&(vec), capacity, sizeof(*(vec)))

#define scc_vec_resize(vec, size)                                       \
    scc_vec_impl_resize(&(vec), size, sizeof(*(vec)))

#define scc_vec_erase(vec, index)                                       \
    scc_vec_impl_erase(vec, index, sizeof(*(vec)))

#define scc_vec_erase_range(vec, first, end)                            \
    scc_vec_impl_erase_range(vec, first, end, sizeof(*(vec)))

#define scc_vec_push(vec, element)                                      \
    (scc_vec_impl_push_ensure_capacity(&(vec), sizeof(*(vec))) &&       \
    ((vec)[scc_vec_impl_base((vec))->sc_size++] = element,1))

#define scc_vec_at(vec, index)                                          \
    ((vec)[scc_vec_impl_at_check(vec, index)])

#define scc_vec_foreach(iter, vec)                                      \
    scc_vec_foreach_by(iter, vec, 1)

#define scc_vec_foreach_reversed(iter, vec)                             \
    scc_vec_foreach_reversed_by(iter, vec, 1)

#define scc_vec_foreach_by(iter, vec, by)                               \
    for(void const *scc_pp_cat_expand(scc_vec_end,__LINE__) =           \
            ((iter) = (vec), &(vec)[scc_vec_size(vec)]);                \
        (void const *)(iter) < scc_pp_cat_expand(scc_vec_end,__LINE__); \
        (iter) += (by))

#define scc_vec_foreach_reversed_by(iter, vec, by)                      \
    for((iter) = &(vec)[scc_vec_size(vec) - 1u]; (iter) >= (vec); (iter) -= (by))

#endif /* SCC_VEC_H */
