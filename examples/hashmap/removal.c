/* Example hashmap removal */

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

    _Bool inserted = scc_hashmap_insert(&map, 32, 0);
    assert(inserted);
    inserted = scc_hashmap_insert(&map, 111, 8);
    assert(inserted);

    /* Look up and print values */
    printf("Value associated with 32: %d\n", *(int **)scc_hashmap_find(map, 32));
    printf("Value associated with 111: %d\n", *(int **)scc_hashmap_find(map, 111));

    _Bool removed = scc_hashmap_remove(map, 111);
    assert(removed);
    printf("Result of looking up 111 after removal: %p\n", scc_hashmap_find(map, 111));

    /* Need to free the map */
    scc_hashmap_free(map);
}

/* ============= OUTPUT =============== */
// STDOUT:Value associated with 32: 0
// STDOUT:Value associated with 111: 8
// STDOUT:Result of looking up 111 after removal: (nil)
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

