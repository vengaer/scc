/* Create a vector with a number of values and pop the last one */

#include <scc/vec.h>

#include <assert.h>
#include <stdio.h>

#ifdef NDEBUG
#error assert does nothing
#endif

int main(void) {
    /* Create an int vec storing the values 1, 2, 3, 4, 5 and 6 */
    scc_vec(int) vec = scc_vec_from(int, 1, 2, 3, 4, 5, 6);

    /* May fail */
    assert(vec);

    /* Check last value */
    printf("Last value in vec: %d\n", vec[scc_vec_size(vec) - 1u]);

    /* Remove the last value */
    int last = scc_vec_pop(vec);
    printf("Poped value: %d\n", last);

    /* Check last value again */
    printf("Last value in vec: %d\n", vec[scc_vec_size(vec) - 1u]);

    /* Free the vec */
    scc_vec_free(vec);
}

/* ============= OUTPUT =============== */
// STDOUT:Last value in vec: 6
// STDOUT:Poped value: 6
// STDOUT:Last value in vec: 5
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
