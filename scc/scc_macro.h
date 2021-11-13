#ifndef SCC_MACRO_H
#define SCC_MACRO_H

#include <stddef.h>

#define scc_offset(type, member)                        \
    (size_t)&(((type *)0)->member)

#define scc_container_qual(addr, type, member, qual)    \
    ((type qual *)((unsigned char qual *)addr - scc_offset(type, member)))

#define scc_container(addr, type, member)               \
    scc_container_qual(addr, type, member,)

#endif /* SCC_MACRO_H */
