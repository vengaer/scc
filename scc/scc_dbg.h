#ifndef SCC_DBG_H
#define SCC_DBG_H

#include <assert.h>

#ifdef NDEBUG
#define scc_bug_on(expr) (void)(expr)
#else
#define scc_bug_on(expr) \
    assert(!(expr))
#endif

#endif /* SCC_DBG_H */
