#ifndef MEMORY_H
#define MEMORY_H

#if __STDC_VERSION__ >= 201112L && !defined __STDC_NO_ATOMICS__
#include <stdatomic.h>
#define scc_clobber()   \
    atomic_signal_fence(memory_order_acq_rel)
#else
#define scc_clobber() \
    do { } while(0)
#endif

#ifdef __clang__
#define scc_do_not_optimize(val)    \
    asm volatile("": "+r,m"(val):: "memory")
#elif defined __GNUC__
#define scc_do_not_optimize(val)    \
    asm volatile("": "+m,r"(val):: "memory")
#else
#define scc_do_not_optimize(val)
#endif

#endif /* MEMORY_H */
