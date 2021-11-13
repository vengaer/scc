#ifndef SCC_VEC_H
#define SCC_VEC_H

#include "scc_macro.h"

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

_Bool scc_vec_impl_push_ensure_capacity(void *vec, size_t elemsize);
_Bool scc_vec_impl_reserve(void *vec, size_t capacity, size_t elemsize);

inline void scc_vec_pop(void *vec) {
    --scc_container(vec, struct scc_vec, sc_buffer)->sc_size;
}

void scc_vec_pop_safe(void *vec);

#define scc_vec_reserve(vec, capacity)              \
    scc_vec_impl_reserve(&(vec), capacity, sizeof(*(vec)))

#define scc_vec_push(vec, element)                  \
    (scc_vec_impl_push_ensure_capacity(&(vec), sizeof(*(vec))) && \
    (vec[scc_container(vec, struct scc_vec, sc_buffer)->sc_size++] = element,1))

#endif /* SCC_VEC_H */
