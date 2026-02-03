/* Example hashmap hash functions */

#include <scc/hashmap.h>

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
 * 64-bit FNV-1
 */
static uint_fast64_t fnv1_64(void const *data, size_t size) {
    uint_fast64_t hash = UINT64_C(0xcbf29ce484222325);
    unsigned char const *dptr = data;
    for(size_t i = 0; i < size; ++i) {
        hash *= UINT64_C(0x100000001b3);
        hash ^= dptr[i];
    }
    return hash;
}

/*
 * Int comparator
 */
static _Bool int_eq(void const *l, void const *r) {
    return *(int const *)l == *(int const *)r;
}

int main(void) {
    extern uint_fast64_t fnv1_64(void const *data, size_t size);
    extern _Bool int_eq(void const *l, void const *r);

    scc_hashmap(int, _Bool) map;
    {
        map = scc_hashmap_with_hash(int, _Bool, int_eq, fnv1_64);
        printf("Initialized map at %p\n", (void *)map);
        /* Map is valid */

    } /* Map invalidated at end of scope */

    /* Map now invalid */
}

/* ============= OUTPUT =============== */
// STDOUT:Initialized map at
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --check-prefix=STDOUT
