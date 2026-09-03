#ifndef SCC_DEQUE_H
#define SCC_DEQUE_H

#include "bits.h"
#include "bug.h"
#include "mem.h"

#include <stddef.h>

#ifndef SCC_DEQUE_STATIC_CAPACITY
#define SCC_DEQUE_STATIC_CAPACITY 32
#endif

#if !scc_bits_is_power_of_2(SCC_DEQUE_STATIC_CAPACITY)
#error Stack capacity must be a power of 2
#endif

#define scc_deque(type) type *

#define scc_deque_iter(type) type *

struct scc_deque_base {
    size_t rd_size;
    size_t rd_capacity;
    size_t rd_begin;
    size_t rd_end;
    unsigned char rd_buffer[];
};

#define scc_deque_impl_layout(type)                                             \
    struct {                                                                    \
        struct {                                                                \
            size_t rd_size;                                                     \
            size_t rd_capacity;                                                 \
            size_t rd_begin;                                                    \
            size_t rd_end;                                                      \
            unsigned char rd_npad;                                              \
            unsigned char rd_dynalloc;                                          \
        } rd0;                                                                  \
        type rd_data[SCC_DEQUE_STATIC_CAPACITY];                                \
    }

#define scc_deque_impl_dataoff(type)                                            \
    sizeof(                                                                     \
        struct {                                                                \
            struct {                                                            \
                size_t rd_size;                                                 \
                size_t rd_capacity;                                             \
                size_t rd_begin;                                                \
                size_t rd_end;                                                  \
                unsigned char rd_npad;                                          \
                unsigned char rd_dynalloc;                                      \
            } rd0;                                                              \
            type rd_data[];                                                     \
        }                                                                       \
    )

#define scc_deque_impl_base_qual(deque, qual)                                   \
    scc_container_qual(                                                         \
        (unsigned char qual *)(deque) - scc_deque_impl_npad(deque),             \
        struct scc_deque_base,                                                  \
        rd_buffer,                                                              \
        qual                                                                    \
    )

#define scc_deque_impl_base(deque)                                              \
    scc_deque_impl_base_qual(deque,)

void *scc_deque_impl_new(struct scc_deque_base *base, size_t offset, size_t capacity);

void *scc_deque_impl_new_dyn(size_t dequesz, size_t offset, size_t capacity);

#define scc_deque_new(type)                                                     \
    (type *)scc_deque_impl_new(                                                 \
        (void *)&(scc_deque_impl_layout(type)) { 0 },                           \
        scc_deque_impl_dataoff(type),                                           \
        SCC_DEQUE_STATIC_CAPACITY                                               \
    )

#define scc_deque_new_dyn(type)                                                 \
    (type *)scc_deque_impl_new_dyn(                                             \
        sizeof(scc_deque_impl_layout(type)),                                    \
        scc_deque_impl_dataoff(type),                                           \
        SCC_DEQUE_STATIC_CAPACITY                                               \
    )

void scc_deque_free(void *deque);

inline size_t scc_deque_impl_npad(void const *deque) {
    return ((unsigned char const *)deque)[-2] + 2 * sizeof(unsigned char);
}

inline size_t scc_deque_capacity(void const *deque) {
    return scc_deque_impl_base_qual(deque, const)->rd_capacity;
}

inline size_t scc_deque_size(void const *deque) {
    return scc_deque_impl_base_qual(deque, const)->rd_size;
}

inline _Bool scc_deque_empty(void const *deque) {
    return !scc_deque_size(deque);
}

_Bool scc_deque_impl_prepare_push(void *dequeaddr, size_t elemsize);

inline size_t scc_deque_impl_push_back_index(void *deque) {
    struct scc_deque_base *base = scc_deque_impl_base(deque);
    size_t index = base->rd_end;
    base->rd_end = (base->rd_end + 1u) & (base->rd_capacity - 1u);
    ++base->rd_size;
    return index;
}

#define scc_deque_push_back(dequeaddr, ...)                                     \
    (scc_deque_impl_prepare_push(dequeaddr, sizeof(**(dequeaddr))) &&           \
    (((*(dequeaddr))[scc_deque_impl_push_back_index(*(dequeaddr))] = __VA_ARGS__),1))

inline size_t scc_deque_impl_push_front_index(void *deque) {
    struct scc_deque_base *base = scc_deque_impl_base(deque);
    base->rd_begin = (base->rd_begin - 1u) & (base->rd_capacity - 1u);
    ++base->rd_size;
    return base->rd_begin;
}

#define scc_deque_push_front(dequeaddr, ...)                                    \
    (scc_deque_impl_prepare_push(dequeaddr, sizeof(**(dequeaddr))) &&           \
    ((*(dequeaddr))[scc_deque_impl_push_front_index(*(dequeaddr))] = __VA_ARGS__),1)

inline size_t scc_deque_impl_pop_back_index(void *deque) {
    struct scc_deque_base *base = scc_deque_impl_base(deque);
    base->rd_end = (base->rd_end - 1u) & (base->rd_capacity - 1u);
    --base->rd_size;
    return base->rd_end;
}

#define scc_deque_pop_back(deque)                                               \
    (deque)[scc_deque_impl_pop_back_index(deque)]

inline size_t scc_deque_impl_pop_front_index(void *deque) {
    struct scc_deque_base *base = scc_deque_impl_base(deque);
    size_t index = base->rd_begin;
    base->rd_begin = (base->rd_begin + 1u) & (base->rd_capacity - 1u);
    --base->rd_size;
    return index;
}

#define scc_deque_pop_front(deque)                                              \
    (deque)[scc_deque_impl_pop_front_index(deque)]

inline size_t scc_deque_impl_back_index(void const *deque) {
    struct scc_deque_base const *base = scc_deque_impl_base_qual(deque, const);
    return (base->rd_end - 1u) & (base->rd_capacity - 1u);
}

#define scc_deque_back(deque)                                                   \
    (deque)[scc_deque_impl_back_index(deque)]

#define scc_deque_front(deque)                                                  \
    (deque)[scc_deque_impl_base_qual(deque, const)->rd_begin]

inline void scc_deque_clear(void *deque) {
    struct scc_deque_base *base = scc_deque_impl_base(deque);
    base->rd_size = 0u;
    base->rd_begin = 0u;
    base->rd_end = 0u;
}

_Bool scc_deque_impl_reserve(void *dequeaddr, size_t capacity, size_t elemsize);

#define scc_deque_reserve(dequeaddr, capacity)                                  \
    scc_deque_impl_reserve(dequeaddr, capacity, sizeof(**(dequeaddr)))

void *scc_deque_impl_clone(void const *deque, size_t elemsize);

#define scc_deque_clone(deque)                                                  \
    scc_deque_impl_clone(deque, sizeof(*(deque)))

inline void *scc_deque_impl_iter_start(void *deque, size_t elemsize) {
    struct scc_deque_base const *base = scc_deque_impl_base_qual(deque, const);
    unsigned char *p = deque;
    return p + elemsize * base->rd_begin;
}

inline void *scc_deque_impl_iter_end(void *deque, size_t elemsize) {
    struct scc_deque_base const *base = scc_deque_impl_base_qual(deque, const);
    unsigned char *p = deque;
    return p + elemsize * base->rd_end;
}

void *scc_deque_impl_iter_next(void *it, void *deque, size_t elemsize);

#define scc_deque_foreach(iter, deque)                                          \
    for (void const *deque_iterend =                                            \
        ((void)((iter) = scc_deque_impl_iter_start(deque, sizeof(*(deque)))),   \
            scc_deque_impl_iter_end(deque, sizeof(*(deque))));                  \
         (iter) != deque_iterend;                                               \
         (iter) = scc_deque_impl_iter_next(iter, deque, sizeof(*(deque))))

#endif /* SCC_DEQUE_H */
