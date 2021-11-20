#ifndef SCC_SVEC_H
#define SCC_SVEC_H

#include "scc_mem.h"

#include <stddef.h>

#ifndef SCC_SVEC_STATIC_CAPACITY
enum { SCC_SVEC_STATIC_CAPACITY = 32 };
#endif

struct scc_svec {
    size_t sc_size;
    size_t sc_capacity;
    unsigned char sc_buffer[];
};

#define scc_svec_impl_layout(type)                          \
    struct {                                                \
        size_t sc_size;                                     \
        size_t sc_capacity;                                 \
        unsigned char sc_npad;                              \
        unsigned char sc_dynalloc;                          \
        type sc_buffer[SCC_SVEC_STATIC_CAPACITY];           \
    }

#define scc_svec_initsize(type)                             \
    sizeof(scc_svec_impl_layout(type))

#define scc_svec_impl_initvec(type)                         \
    (union {                                                \
        struct scc_svec sc_svec;                            \
        unsigned char sc_buffer[scc_svec_initsize(type)];   \
    }){ 0 }.sc_svec

#define scc_svec_impl_base_qual(svec, qual)                 \
    scc_container_qual(svec - scc_svec_impl_npad(svec), struct scc_svec, sc_buffer, qual)

#define scc_svec_impl_base(svec)                            \
    scc_svec_impl_base_qual(svec,)

void *scc_svec_impl_init(void *initvec, size_t offset, size_t capacity);

inline size_t scc_svec_impl_npad(void const *svec) {
    return ((unsigned char const *)svec)[-2] + 2 * sizeof(unsigned char);
}

#define scc_svec_init(type)                                 \
    scc_svec_impl_init(&scc_svec_impl_initvec(type), offsetof(scc_svec_impl_layout(type), sc_buffer), SCC_SVEC_STATIC_CAPACITY)

void scc_svec_free(void *svec);

#endif /* SCC_SVEC_H */