/* Example btree comparators */

#include <scc/btree.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef NDEBUG
#error assert does nothing
#endif

/*
 * Int comparator function
 */
static int compare_int(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

/*
 * strcmp works well enough for strings
 */
static int compare_str(void const *l, void const *r) {
    return strcmp(l, r);
}

int main(void) {
    printf("compare_int(&(int){ 0 }, &(int){ 3 }): %d\n",
        compare_int(&(int){ 0 }, &(int){ 3 }));
    printf("compare_int(&(int){ 81 }, &(int){ 7 }): %d\n",
        compare_int(&(int){ 81 }, &(int){ 7 }));
    printf("compare_int(&(int){ 33 }, &(int){ 33 }): %d\n",
        compare_int(&(int){ 33 }, &(int){ 33 }));

    printf("compare_str(\"as\", \"ef\") < 0: %d\n",
        compare_str("as", "ef") < 0);
    printf("compare_str(\"str\", \"ef\") > 0: %d\n",
        compare_str("str", "ef") > 0);
    printf("compare_str(\"str\", \"str\") == 0: %d\n",
        !compare_str("str", "str"));
}

/* ============= OUTPUT =============== */
// STDOUT:compare_int(&(int){ 0 }, &(int){ 3 }): -3
// STDOUT:compare_int(&(int){ 81 }, &(int){ 7 }): 74
// STDOUT:compare_int(&(int){ 33 }, &(int){ 33 }): 0
// STDOUT:compare_str("as", "ef") < 0: 1
// STDOUT:compare_str("str", "ef") > 0: 1
// STDOUT:compare_str("str", "str") == 0: 1
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

