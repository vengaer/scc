/* Modify value in hashmap after lookup */

#include <scc/hashmap.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef NDEBUG
#error assert has not effect
#endif

/*
 * Int comparator
 */
static _Bool int_eq(void const *l, void const *r) {
    return *(int const *)l == *(int const *)r;
}

int main(void) {
    extern _Bool int_eq(void const *l, void const *r);

    scc_hashmap(int, int) map = scc_hashmap_new(int, int, int_eq);

    _Bool inserted = scc_hashmap_insert(&map, 100, 200);
    assert(inserted);
    inserted = scc_hashmap_insert(&map, 200, 400);
    assert(inserted);

    printf("Value associated with 100: %d\n", *(int **)scc_hashmap_find(map, 100));
    printf("Value associated with 200: %d\n", *(int **)scc_hashmap_find(map, 200));

    int *val = scc_hashmap_find(map, 200);
    /* Modify the value */
    *val = 1234;

    printf("Value associated with 200: %d\n", *(int **)scc_hashmap_find(map, 200));

    scc_hashmap_free(map);
}

/* ============= OUTPUT =============== */
// STDOUT:Value associated with 100: 200
// STDOUT:Value associated with 200: 400
// STDOUT:Value associated with 200: 1234
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

