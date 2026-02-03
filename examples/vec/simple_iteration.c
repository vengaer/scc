/* Iterate over a vector */

#include <scc/vec.h>

#include <assert.h>
#include <stdio.h>

#ifdef NDEBUG
#error assert does nothing
#endif

int main(void) {
    /* Create an int vec */
    scc_vec(int) vec = scc_vec_new(int);

    /* May fail */
    assert(vec);

    /* Push the values 68, 3 and 12 */
    assert(scc_vec_push(&vec, 68));
    assert(scc_vec_push(&vec, 3));
    assert(scc_vec_push(&vec, 12));

    /* Create an iterator */
    scc_vec_iter(int) it;

    /* Iterate over the vec and print each element */
    scc_vec_foreach(it, vec)
        printf("%d\n", *it);

    /* Free the vec */
    scc_vec_free(vec);
}

/* ============= OUTPUT =============== */
// STDOUT:68
// STDOUT:3
// STDOUT:12
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
