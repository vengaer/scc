#ifndef SCC_PERF_H
#define SCC_PERF_H

/* SCC_ON_PERFTRACK
 *
 * Internal use only
 *
 * Hack for executing arbitrary statements
 * if SCC_PERFEVTS is defined without having
 * to riddle the code with ifdefs
 *
 * ...
 *      The expression to execute
 */
#ifdef SCC_PERFEVTS
#define SCC_ON_PERFTRACK(...)   \
    do {                        \
        __VA_ARGS__;            \
    } while (0)
#else
#define SCC_ON_PERFTRACK(...) do { } while (0)
#endif

#endif /* SCC_PERF_H */
