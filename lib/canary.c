#include <scc/canary.h>

#include <stdbool.h>

#define CANARY_INITIAL 0xd4u

#ifdef SCC_CANARY_ENABLED
static inline unsigned char scc_canary_next_byte(unsigned char n) {
    return (n << 1) ^ n;
}

void scc_canary_init(unsigned char *canary, size_t size) {
    unsigned char c = CANARY_INITIAL;
    for (unsigned i = 0u; i < size; ++i) {
        canary[i] = c;
        c = scc_canary_next_byte(c);
    }
}

_Bool scc_canary_intact(unsigned char const *canary, size_t size) {
    unsigned char c = CANARY_INITIAL;
    bool intact = true;
    for (unsigned i = 0u; i < size && intact; ++i) {
        intact &= (canary[i] == c);
        c = scc_canary_next_byte(c);
    }

    return intact;
}
#endif
