#ifndef SCC_SVEC_H
#define SCC_SVEC_H

#include "scc_mem.h"

#include <stddef.h>

#ifndef SCC_SVEC_STATIC_CAPACITY
enum { SCC_SVEC_STATIC_CAPACITY = 32 };
#endif

#define scc_svec(type) type *

struct scc_svec {
    size_t sc_size;
    size_t sc_capacity;
    unsigned char sc_buffer[];
};

#define scc_svec_impl_layout(type)                                      \
    struct {                                                            \
        size_t sc_size;                                                 \
        size_t sc_capacity;                                             \
        unsigned char sc_npad;                                          \
        unsigned char sc_dynalloc;                                      \
        type sc_buffer[SCC_SVEC_STATIC_CAPACITY];                       \
    }

#define scc_svec_initsize(type)                                         \
    sizeof(scc_svec_impl_layout(type))

#define scc_svec_impl_initvec(type)                                     \
    (union {                                                            \
        struct scc_svec sc_svec;                                        \
        unsigned char sc_buffer[scc_svec_initsize(type)];               \
    }){ 0 }.sc_svec

#define scc_svec_impl_base_qual(svec, qual)                             \
    scc_container_qual(                                                 \
        svec - scc_svec_impl_npad(svec),                                \
        struct scc_svec,                                                \
        sc_buffer,                                                      \
        qual                                                            \
    )

#define scc_svec_impl_base(svec)                                        \
    scc_svec_impl_base_qual(svec,)

#define scc_svec_impl_offset(type)                                      \
    offsetof(scc_svec_impl_layout(type), sc_buffer)

void *scc_svec_impl_init(void *initvec, size_t offset, size_t capacity);
void *scc_svec_impl_from(
        void *restrict vec,
        void const *restrict data,
        size_t size,
        size_t elemsize
    );
_Bool scc_svec_impl_resize(void *vec, size_t size, size_t elemsize);
void scc_svec_impl_erase(void *svec, size_t index, size_t elemsize);
void scc_svec_impl_erase_range(void *svec, size_t first, size_t end, size_t elemsize);
void scc_svec_impl_at_check(void *svec, size_t index);
_Bool scc_svec_impl_push_ensure_capacity(void *vec, size_t elemsize);
_Bool scc_svec_impl_reserve(void *vec, size_t capacity, size_t elemsize);

inline size_t scc_svec_impl_npad(void const *svec) {
    return ((unsigned char const *)svec)[-2] + 2 * sizeof(unsigned char);
}

#define scc_svec_init(type)                                             \
    scc_svec_impl_init(                                                 \
        &scc_svec_impl_initvec(type),                                   \
        scc_svec_impl_offset(type),                                     \
        SCC_SVEC_STATIC_CAPACITY                                        \
    )

#define scc_svec_from(type, ...)                                        \
    scc_svec_impl_from(                                                 \
        scc_svec_init(type),                                            \
        (type[]){ __VA_ARGS__ },                                        \
        scc_arrsize(((type[]){ __VA_ARGS__ })),                         \
        sizeof(type)                                                    \
    )

void scc_svec_free(void *svec);

inline size_t scc_svec_size(void const *svec) {
    return scc_svec_impl_base_qual(svec, const)->sc_size;
}

inline size_t scc_svec_capacity(void const *svec) {
    return scc_svec_impl_base_qual(svec, const)->sc_capacity;
}

inline _Bool scc_svec_empty(void const *svec) {
    return !scc_svec_size(svec);
}

inline void scc_svec_pop(void *svec) {
    --scc_svec_impl_base(svec)->sc_size;
}

void scc_svec_pop_safe(void *vec);

inline void scc_svec_clear(void *vec) {
    scc_svec_impl_base(vec)->sc_size = 0u;
}

#define scc_svec_reserve(svec, capacity)                                \
    scc_svec_impl_reserve(&(svec), capacity, sizeof(*(svec)))

#define scc_svec_resize(svec, size)                                     \
    scc_svec_impl_resize(&(svec), size, sizeof(*(svec)))

#define scc_svec_erase(svec, index)                                     \
    scc_svec_impl_erase(svec, index, sizeof(*(svec)))

#define scc_svec_erase_range(svec, first, end)                          \
    scc_svec_impl_erase_range(svec, first, end, sizeof(*(svec)))

#define scc_svec_push(svec, element)                                    \
    (scc_svec_impl_push_ensure_capacity(&(svec), sizeof(*(svec))) &&    \
    (svec[scc_svec_impl_base(svec)->sc_size++] = element,1))

#define scc_svec_at(svec, index)                                        \
    (scc_svec_impl_at_check(svec, index),svec[index])

#define scc_svec_foreach(iter, svec)                                    \
    scc_svec_foreach_by(iter, svec, 1)

#define scc_svec_foreach_reversed(iter, svec)                           \
    scc_svec_foreach_reversed_by(iter, svec, 1)

#define scc_svec_foreach_by(iter, svec, by)                             \
    for(iter = svec; iter < &svec[scc_svec_size(svec)]; iter += by)

#define scc_svec_foreach_reversed_by(iter, svec, by)                    \
    for(iter = &svec[scc_svec_size(svec) - 1u]; iter >= svec; iter -= by)

#endif /* SCC_SVEC_H */
