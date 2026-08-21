#ifndef SCC_CANARY_H
#define SCC_CANARY_H

#include "pp_token.h"

#include <stddef.h>

#ifdef SCC_CANARY_ENABLED
#define SCC_CANARY_INJECT(size) unsigned char scc_pp_cat_expand(canary,__LINE__)[size];

void scc_canary_init(unsigned char *canary, size_t size);

_Bool scc_canary_intact(unsigned char const *canary, size_t size);
#else
#define SCC_CANARY_INJECT(...)
#define scc_canary_init(...) ((void)0)
#define scc_canary_intact(...) (_Bool)1
#endif


#endif /* SCC_CANARY_H */
