#ifndef SCC_MURMUR32_H
#define SCC_MURMUR32_H

#include <scc/config.h>

#include <stdint.h>

struct scc_digest128;

#ifdef SCC_HAVE_UINT32_T
void scc_murmur32_128(struct scc_digest128 *digest, void const *data,
        size_t size, uint_fast32_t seed);
#endif


#endif /* SCC_MURMUR32_H */
