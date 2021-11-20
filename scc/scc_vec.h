#ifndef SCC_VEC_H
#define SCC_VEC_H

#include "scc_mem.h"

#include <stddef.h>

struct scc_vec {
    size_t sc_size;
    size_t sc_capacity;
    unsigned char sc_buffer[];
};

void *scc_vec_init(void);
void scc_vec_free(void *vec);

inline size_t scc_vec_size(void const *vec) {
    return scc_container_qual(vec, struct scc_vec, sc_buffer, const)->sc_size;
}

inline size_t scc_vec_capacity(void const *vec) {
    return scc_container_qual(vec, struct scc_vec, sc_buffer, const)->sc_capacity;
}

inline _Bool scc_vec_empty(void const *vec) {
    return !scc_vec_size(vec);
}

inline void scc_vec_pop(void *vec) {
    --scc_container(vec, struct scc_vec, sc_buffer)->sc_size;
}

inline void scc_vec_clear(void *vec) {
    scc_container(vec, struct scc_vec, sc_buffer)->sc_size = 0u;
}

_Bool scc_vec_impl_resize(void *vec, size_t size, size_t elemsize);
_Bool scc_vec_impl_push_ensure_capacity(void *vec, size_t elemsize);
_Bool scc_vec_impl_reserve(void *vec, size_t capacity, size_t elemsize);
void scc_vec_impl_erase(void *vec, void *iter, size_t elemsize);
void scc_vec_impl_erase_range(void *vec, void *first, void *end, size_t elemsize);
void scc_vec_impl_at_check(void *vec, size_t index);

void scc_vec_pop_safe(void *vec);

#define scc_vec_reserve(vec, capacity)              \
    scc_vec_impl_reserve(&(vec), capacity, sizeof(*(vec)))

#define scc_vec_resize(vec, size)                   \
    scc_vec_impl_resize(&(vec), size, sizeof(*(vec)))

#define scc_vec_erase(vec, iter)                    \
    scc_vec_impl_erase(vec, iter, sizeof(*(vec)))

#define scc_vec_erase_range(vec, first, end)        \
    scc_vec_impl_erase_range(vec, first, end, sizeof(*(vec)))

#define scc_vec_push(vec, element)                  \
    (scc_vec_impl_push_ensure_capacity(&(vec), sizeof(*(vec))) && \
    (vec[scc_container(vec, struct scc_vec, sc_buffer)->sc_size++] = element,1))

#define scc_vec_at(vec, index)                      \
    (scc_vec_impl_at_check(vec, index),vec[index])

#define scc_vec_foreach(iter, vec)                  \
    scc_vec_foreach_by(iter, vec, 1)

#define scc_vec_foreach_reversed(iter, vec)         \
    scc_vec_foreach_reversed_by(iter, vec, 1)

#define scc_vec_foreach_by(iter, vec, by)           \
    for(iter = vec; iter < &vec[scc_vec_size(vec)]; iter += by)

#define scc_vec_foreach_reversed_by(iter, vec, by)  \
    for(iter = &vec[scc_vec_size(vec) - 1u]; iter >= vec; iter -= by)

#endif /* SCC_VEC_H */
