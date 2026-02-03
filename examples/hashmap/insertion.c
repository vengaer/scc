/* Example hashmap insertion */

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

    scc_hashmap(int, char const *) map = scc_hashmap_new(int, char const *, int_eq);

    /* scc_hashmap_insert takes the pointer to the map handle, not the handle itself */
    _Bool inserted = scc_hashmap_insert(&map, 32, "Thirty two");
    assert(inserted);

    /* Insert another value */
    inserted = scc_hashmap_insert(&map, 111, "One hundred eleven");
    assert(inserted);

    /* Look up and print values */
    printf("Value associated with 32: %s\n", *(char const **)scc_hashmap_find(map, 32));
    printf("Value associated with 111: %s\n", *(char const **)scc_hashmap_find(map, 111));
    /* NULL is returned whenever a key is not found */
    printf("Value associated with 123: %p\n", scc_hashmap_find(map, 123));

    /* Need to free the map */
    scc_hashmap_free(map);
}

/* ============= OUTPUT =============== */
// STDOUT:Value associated with 32: Thirty two
// STDOUT:Value associated with 111: One hundred eleven
// STDOUT:Value associated with 123: (nil)
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
