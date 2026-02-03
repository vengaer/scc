/* Insert pairs in a btmap */

#include <scc/btmap.h>

#include <assert.h>
#include <stdio.h>

#ifdef NDEBUG
#error assert does nothing
#endif

/* Comparator adheres to the pattern used by strcmp,
 * memcmp, etc., i.e.
 *
 * - Negative return value implies that *l < *r
 * - Positive return value implies that *l > *r
 * - Return value 0 implies that *l == *r
 */
static int compare_int(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

int main(void) {
    /* Instantiate the map */
    scc_btmap(int, int) btmap = scc_btmap_new(int, int, compare_int);

    /* Insert first pair */
    assert(scc_btmap_insert(&btmap, 1, 2));

    int *p = scc_btmap_find(btmap, 1);
    assert(p);
    printf("Value associated with 1: %d\n", *p);

    /* Insert second pair */
    assert(scc_btmap_insert(&btmap, 2, 3));

    p = scc_btmap_find(btmap, 2);
    assert(p);
    printf("Value associated with 2: %d\n", *p);

    /* Overwrite value associated with 1 */
    assert(scc_btmap_insert(&btmap, 1, 8));

    p = scc_btmap_find(btmap, 1);
    assert(p);
    printf("Value associated with 1: %d\n", *p);

    /* Free the map */
    scc_btmap_free(btmap);
}

/* ============= OUTPUT =============== */
// STDOUT:Value associated with 1: 2
// STDOUT:Value associated with 2: 3
// STDOUT:Value associated with 1: 8
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
