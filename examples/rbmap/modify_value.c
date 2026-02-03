/* Example rbmap insertion */

#include <scc/rbmap.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef NDEBUG
#error assert has not effect
#endif

/*
 * Int comparator
 */
static int compare_int(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

int main(void) {
    extern int compare_int(void const *l, void const *r);

    /* Create an instance mapping ints to ints */
    scc_rbmap(int, int) map = scc_rbmap_new(int, int, compare_int);

    /* Insert the pair (32, 8) */
    assert(scc_rbmap_insert(&map, 32, 8));

    /* Look up the value */
    int *p = scc_rbmap_find(map, 32);
    assert(p);

    printf("Value before modification: %d\n", *p);

    /* Modify the value */
    *p = 1;

    /* Look it up again */
    int *pp = scc_rbmap_find(map, 32);
    assert(pp);

    printf("Value after modification: %d\n", *pp);

    /* Free the instance */
    scc_rbmap_free(map);
}

/* ============= OUTPUT =============== */
// STDOUT:Value before modification: 8
// STDOUT:Value after modification: 1
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
