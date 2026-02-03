/* Example stack pushing and poping */

#include <scc/stack.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef NDEBUG
#error assert has not effect
#endif

int main(void) {
    /* Create an instance containing ints */
    scc_stack(int) stack = scc_stack_new(int);

    /* Insert values */
    _Bool inserted = true;
    for (int i = 0; i < 10; ++i)
        inserted &= scc_stack_push(&stack, i);
    assert(inserted);

    /* Pop and print values */
    while (!scc_stack_empty(stack))
        printf("%d\n", scc_stack_pop(stack));


    /* Free the instance */
    scc_stack_free(stack);
}

/* ============= OUTPUT =============== */
// STDOUT:9
// STDOUT:8
// STDOUT:7
// STDOUT:6
// STDOUT:5
// STDOUT:4
// STDOUT:3
// STDOUT:2
// STDOUT:1
// STDOUT:0
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
