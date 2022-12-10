#ifndef SCC_BITS_H
#define SCC_BITS_H

#define scc_bits_is_power_of_2(val)     \
    ((val) && (((val) & ~((val) - 1)) == (val)))

#define scc_bits_is_even(val)           \
    (!((val) & 0x01))

#endif /* SCC_BITS_H */
