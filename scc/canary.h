#ifndef CANARY_H
#define CANARY_H

#include "pp_token.h"

#include <stddef.h>

#ifdef SCC_CANARY_ENABLED
//? .. c:macro:: SCC_CANARY_INJECT(size)
//?
//?     Inject a canary of the given size in a struct. The
//?     data must be initialized using :ref:`scc_canary_init <scc_canary_init>`
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param size: The size of the canary. Must be an ICE
#define SCC_CANARY_INJECT(size) unsigned char scc_pp_cat_expand(canary,__LINE__)[size];

//? .. _scc_canary_init:
//? .. c:function:: void scc_canary_init(unsigned char *canary, size_t size)
//?
//?     Initialize a canary of the specified size at the given address
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param canary: Address of the canary
//?     :param size: Size of the canary
void scc_canary_init(unsigned char *canary, size_t size);

//? .. c:function:: _Bool scc_canary_intact(unsigned char *canary, size_t size)
//?
//?     Verify that the given canary remains intact
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param canary: Address of the canary
//?     :param size: Size of the canary
//?     :returns: :code:`true` if the canary remains intact, otherwise
//?               :code:`false`
_Bool scc_canary_intact(unsigned char const *canary, size_t size);
#else
#define SCC_CANARY_INJECT(...)
#define scc_canary_init(...) ((void)0)
#define scc_canary_intact(...) (_Bool)1
#endif


#endif /* CANARY_H */
