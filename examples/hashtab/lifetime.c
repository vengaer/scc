/* Example hashtab initialization */

#include <scc/hashtab.h>

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

    /* Map invalid */
    scc_hashtab(int) tab;
    {
        tab = scc_hashtab_with_hash(int, int_eq, fnv1_64);
        printf("Initialized table at %p\n", (void *)tab);
        /* tab is valid */
    } /* invalidated at scope end */

    /* tab is no longer valid */
}

/* ============= OUTPUT =============== */
// STDOUT:Initialized table at
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --check-prefix=STDOUT
