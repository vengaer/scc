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

    /* Insert a couple of pairs */
    _Bool inserted = true;
    for (int i = 0; i < 4; ++i)
        inserted &= scc_rbmap_insert(&map, i, 2 * i);
    assert(inserted);

    /* Look up and print values */
    for (unsigned i = 0u; i < scc_rbmap_size(map); ++i)
        printf("Value associated with %u: %d\n", i, *(int *)scc_rbmap_find(map, (int)i));

    /* Free the instance */
    scc_rbmap_free(map);
}

/* ============= OUTPUT =============== */
// STDOUT:Value associated with 0: 0
// STDOUT:Value associated with 1: 2
// STDOUT:Value associated with 2: 4
// STDOUT:Value associated with 3: 6
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
