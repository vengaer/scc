#ifndef DEBUG_H
#define DEBUG_H

#include <scc/scc_pp_token.h>

#include <stdio.h>

#ifdef SCC_FUZZ_DEBUG

#define dbg_pr(...) printf(__VA_ARGS__)
#define dbg_pr_n(it, data, n, ...)                      \
    for((it) = &(data)[0]; (it) < &(data)[n]; ++(it)) { \
        dbg_pr(__VA_ARGS__);                            \
    }

#else
#define dbg_pr(...) do { } while(0)
#define dbg_pr_n(it, ...) (void)it
#endif

#endif /* DEBUG_H */
