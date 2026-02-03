/* Create a btmap instance mapping ints to a user-defined struct */

#include <scc/btmap.h>

#include <assert.h>
#include <stdio.h>

#ifdef NDEBUG
#error assert does nothing
#endif

/* Arbitrary struct to store as values */
struct foo {
    int a;
    char const b[32];
};

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
    /* Create a btmap instance */
    scc_btmap(int, struct foo) btmap = scc_btmap_new(int, struct foo, compare_int);

    /* The map has the default order */
    assert(scc_btmap_order(btmap) == SCC_BTMAP_DEFAULT_ORDER);

    /* Print the order */
    printf("Tree order: %zu\n", scc_btmap_order(btmap));

    /* Free the map */
    scc_btmap_free(btmap);
}

/* ============= OUTPUT =============== */
// STDOUT:Tree order: 6
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
