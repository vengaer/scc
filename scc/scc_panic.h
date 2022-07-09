#ifndef SCC_PANIC_H
#define SCC_PANIC_H

#include "scc_pp_token.h"

#include <stdio.h>
#include <stdlib.h>

#define scc_panic_impl(fmt, ...)                                        \
    do {                                                                \
        (void)fprintf(stderr, "SCC panic at %s:%s: " fmt "%s\n",        \
                __FILE__, scc_pp_str_expand(__LINE__), __VA_ARGS__);    \
        exit(1);                                                        \
    } while(0)

#define scc_panic(...)  \
    scc_panic_impl(__VA_ARGS__, "")

#endif /* SCC_PANIC_H */
