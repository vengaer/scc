#include <scc/canary.h>

#include <stdbool.h>

#define CANARY_INITIAL 0xd4u

#ifdef SCC_CANARY_ENABLED
//? .. c:function:: scc_canary_next_byte(unsigned char current)
//?
//?     Given byte n % 8, generates byte (n + 1) % 8 in the seuqence
//?     0xd4, 0x7c, 0x84, 0x8c, 0x9f, 0xbc, 0xc4, 0x4c
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param n: The nth byte, used to compute the next
//?     :returns: The n+1th byte in the above sequence
static inline unsigned char scc_canary_next_byte(unsigned char n) {
    return (n << 1) ^ n;
}

void scc_canary_init(unsigned char *canary, size_t size) {
    unsigned char c = CANARY_INITIAL;
    for(unsigned i = 0u; i < size; ++i) {
        canary[i] = c;
        c = scc_canary_next_byte(c);
    }
}

_Bool scc_canary_intact(unsigned char const *canary, size_t size) {
    unsigned char c = CANARY_INITIAL;
    bool intact = true;
    for(unsigned i = 0u; i < size && intact; ++i) {
        intact &= (canary[i] == c);
        c = scc_canary_next_byte(c);
    }

    return intact;
}
#endif
