/* Iterate over a vector */

#include <scc/vec.h>

#include <assert.h>
#include <stdio.h>

#ifdef NDEBUG
#error assert does nothing
#endif

int main(void) {
    /* Create an int vec containing the values 1 through 6*/
    scc_vec(int) vec = scc_vec_from(int, 1, 2, 3, 4, 5, 6);

    /* May fail */
    assert(vec);

    /* Create an iterator */
    scc_vec_iter(int) it;

    /* Iterate over every other element in the vec and print it */
    scc_vec_foreach_by(it, vec, 2)
        printf("%d\n", *it);

    /* Free the vec */
    scc_vec_free(vec);
}

/* ============= OUTPUT =============== */
// STDOUT:1
// STDOUT:3
// STDOUT:5
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
