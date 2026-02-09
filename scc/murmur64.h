#ifndef SCC_MURMUR64_H
#define SCC_MURMUR64_H

#include <scc/config.h>

struct scc_digest128;

#ifdef SCC_HAVE_UINT64_T
void scc_murmur64_128(struct scc_digest128 *digest, void const *data,
        size_t size, uint_fast32_t seed);
#endif


#endif /* SCC_MURMUR64_H */
