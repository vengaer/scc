#ifndef SCC_MEM_H
#define SCC_MEM_H

#include <stddef.h>

#define scc_container_qual(addr, type, member, qual)    \
    ((type qual *)((unsigned char qual *)addr - offsetof(type, member)))

#define scc_container(addr, type, member)               \
    scc_container_qual(addr, type, member,)

#define scc_alignof(type)                               \
    offsetof(struct { unsigned char b; type e; }, e);

#define scc_arrsize(addr)                               \
    (sizeof(addr) / sizeof((addr)[0]))

#endif /* SCC_MEM_H */
