#ifndef SCC_SWAR_H
#define SCC_SWAR_H

#include "bug.h"

#include <assert.h>
#include <limits.h>
#include <string.h>

typedef unsigned long long scc_vectype;

 * at least 8 bits */
#if CHAR_BIT < 8
#error Non-conformant implementation
#endif

inline unsigned char scc_swar_read_byte(scc_vectype vec, unsigned i) {
    assert(i < sizeof(vec));
    return (vec >> i * CHAR_BIT) & UCHAR_MAX;
}

inline scc_vectype scc_swar_bcast(unsigned char byte) {
    /* Cannot assume sizeof(scc_vectype) */
    scc_vectype mask = 0u;
    for (unsigned i = 0u; i < sizeof(mask); ++i) {
        scc_when_mutating(assert(i < sizeof(mask)));
        mask = (mask << CHAR_BIT) | 0x01u;
    }

    /* Broadcast to all bytes in the vector */
    return mask * byte;
}

inline scc_vectype const *scc_swar_align_load(unsigned char const *ldaddr) {
    unsigned char byte;
    memcpy(&byte, &ldaddr, sizeof(byte));
    unsigned char aligned = byte & ~(sizeof(scc_vectype) - 1u);
    return (void const *)(ldaddr + aligned - byte);
}

#endif /* SCC_SWAR_H */
