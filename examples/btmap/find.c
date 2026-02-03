/* Look up and modify values in btmap */

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
    /* Instantiate a map with order 32 */
    scc_btmap(int, int) btmap = scc_btmap_with_order(int, int, compare_int, 32);

    /* Insert a couple of values */
    for (int i = 0; i < 10; ++i)
        assert(scc_btmap_insert(&btmap, i, 2 * i));

    /* Look up the value associated with 9 */
    int *p = scc_btmap_find(btmap, 9);
    assert(p);
    printf("Value associated with 9: %d\n", *p);

    /* Associated value may be modified through returned pointer */
    *p = 32;

    /* Look it up again */
    p = scc_btmap_find(btmap, 9);
    assert(p);
    printf("Value associated with 9 after modification: %d\n", *p);

    /* Free the map */
    scc_btmap_free(btmap);
}

/* ============= OUTPUT =============== */
// STDOUT:Value associated with 9: 18
// STDOUT:Value associated with 9 after modification: 32
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

