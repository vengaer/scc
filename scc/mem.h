#ifndef SCC_MEM_H
#define SCC_MEM_H

#include <assert.h>
#include <stddef.h>

#define scc_container_qual(addr, type, member, qual)    \
    ((type qual *)((unsigned char qual *)(addr) - offsetof(type, member)))

#define scc_container(addr, type, member)               \
    scc_container_qual(addr, type, member,)

#define scc_alignof(type)                               \
    sizeof(struct { unsigned char b; type e[]; })

#define scc_arrsize(addr)                               \
    (sizeof(addr) / sizeof((addr)[0]))

#define scc_align(addr, bound)                          \
    (((addr) + (bound) - 1u) & ~((bound) - 1u))

#ifdef SCC_INTERCEPT_NULLSIZE_COPIES

#ifdef NDEBUG
#error INTERCEPT_NULLSIZE_COPIES has no effect with NDEBUG defined
#endif

#define scc_memmove(dst, src, n)                        \
    (assert((n) > 0u), memmove((dst), (src), (n)))

#define scc_memcpy(dst, src, n)                         \
    (assert((n) > 0u), memcpy((dst), (src), (n)))

#else

#define scc_memmove(...) memmove(__VA_ARGS__)
#define scc_memcpy(...) memcpy(__VA_ARGS__)

#endif

#endif /* SCC_MEM_H */
