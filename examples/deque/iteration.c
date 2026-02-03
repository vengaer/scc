/* Iterate over a deque */

#include <scc/deque.h>

#include <assert.h>
#include <stdio.h>

#ifdef NDEBUG
#error assert does nothing
#endif

int main(void) {
    /* Create the original deque */
    scc_deque(int) deque = scc_deque_new(int);

    /* Push 32 values */
    for (int i = 0; i < 8; i += 2u)
        assert(scc_deque_push_back(&deque, i));

    /* Create an iterator */
    scc_deque_iter(int) it;

    /* Iterate and print each element */
    scc_deque_foreach(it, deque)
        printf("Element: %d\n", *it);

    /* Free the original instance */
    scc_deque_free(deque);
}

/* ============= OUTPUT =============== */
// STDOUT:Element: 0
// STDOUT:Element: 2
// STDOUT:Element: 4
// STDOUT:Element: 6
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
