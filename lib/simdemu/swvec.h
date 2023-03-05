#ifndef SWVEC_H
#define SWVEC_H

#include <scc/bug.h>

#include <assert.h>
#include <limits.h>
#include <string.h>

//? .. c:type:: unsigned long long scc_vectype
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     Vector type used in simd emulation
typedef unsigned long long scc_vectype;

scc_static_assert(sizeof(scc_vectype) == SCC_VECSIZE, "Unexpected vector size");

/* Standard mandates that UCHAR_MAX >= 255 meaning a byte must be
 * at least 8 bits */
scc_static_assert(CHAR_BIT >= 8, "Non-conformant implementation");

//? .. c:function:: unsigned char scc_swvec_read_byte(scc_vectype vec, unsigned i)
//?
//?     Read the ith byte in the given vector
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param vec: The vector to read from
//?     :param i: The index of the byte to read
//?     :returns: The ith byte of the given vector
inline unsigned char scc_swvec_read_byte(scc_vectype vec, unsigned i) {
    assert(i < sizeof(vec));
    return (vec >> i * CHAR_BIT) & UCHAR_MAX;
}

//? .. c:function:: scc_vectype scc_swvec_bcast(unsigned char byte)
//?
//      Broadcast byte to each byte in a vector and return the latter
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param byte: The byte to broadcast
//?     :returns: A vector where each byte contains the given byte
inline scc_vectype scc_swvec_bcast(unsigned char byte) {
    /* Cannot assume sizeof(scc_vectype) */
    scc_vectype mask = 0u;
    for(unsigned i = 0u; i < sizeof(mask); ++i) {
        scc_when_mutating(assert(i < sizeof(mask)));
        mask = (mask << CHAR_BIT) | 0x01u;
    }

    /* Broadcast to all bytes in the vector */
    return mask * byte;
}

//? .. c:function:: scc_vectype const *scc_swvec_align_load(unsigned char const *ldaddr)
//?
//?     Align :code:`ldaddr` down to the nearest multiple of :c:texpr:`sizeof(scc_vectype)`
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param ldaddr: The address to be aligned
//?     :returns: :code:`ldaddr` rounded down to the nearest multiple
//?               of :c:texpr:`sizeof(scc_vectype)`
inline scc_vectype const *scc_swvec_align_load(unsigned char const *ldaddr) {
    unsigned char byte;
    memcpy(&byte, &ldaddr, sizeof(byte));
    unsigned char aligned = byte & ~(sizeof(scc_vectype) - 1u);
    return (void const *)(ldaddr + aligned - byte);
}

#endif /* SWVEC_H */
