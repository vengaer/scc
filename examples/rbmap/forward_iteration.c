/* Example rbmap iteration */

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
    for (int i = 0; i < 6; ++i)
        inserted &= scc_rbmap_insert(&map, i, 2 * i);
    assert(inserted);

    scc_rbmap_iter(int, int) it;
    unsigned i = 0u;

    scc_rbmap_foreach(it, map)
        printf("%u: key %d value %d\n", i++, it->key, it->value);

    /* Free the instance */
    scc_rbmap_free(map);
}

/* ============= OUTPUT =============== */
// STDOUT:0: key 0 value 0
// STDOUT:1: key 1 value 2
// STDOUT:2: key 2 value 4
// STDOUT:3: key 3 value 6
// STDOUT:4: key 4 value 8
// STDOUT:5: key 5 value 10
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
