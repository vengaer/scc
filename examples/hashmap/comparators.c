/* Example hashmap comparators */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef NDEBUG
#error assert does nothing
#endif

/*
 * Int comparator function
 */
static int int_eq(void const *l, void const *r) {
    return *(int const *)l == *(int const *)r;
}

/*
 * strcmp works well enough for strings
 */
static int str_eq(void const *l, void const *r) {
    return !strcmp(l, r);
}

int main(void) {
    printf("int_eq(&(int){ 0 }, &(int){ 3 }): %d\n",
        int_eq(&(int){ 0 }, &(int){ 3 }));
    printf("int_eq(&(int){ 81 }, &(int){ 7 }): %d\n",
        int_eq(&(int){ 81 }, &(int){ 7 }));
    printf("int_eq(&(int){ 33 }, &(int){ 33 }): %d\n",
        int_eq(&(int){ 33 }, &(int){ 33 }));

    printf("str_eq(\"as\", \"ef\"): %d\n",
        str_eq("as", "ef") < 0);
    printf("str_eq(\"str\", \"ef\"): %d\n",
        str_eq("str", "ef") > 0);
    printf("str_eq(\"str\", \"str\"): %d\n",
        str_eq("str", "str"));
}

/* ============= OUTPUT =============== */
// STDOUT:int_eq(&(int){ 0 }, &(int){ 3 }): 0
// STDOUT:int_eq(&(int){ 81 }, &(int){ 7 }): 0
// STDOUT:int_eq(&(int){ 33 }, &(int){ 33 }): 1
// STDOUT:str_eq("as", "ef"): 0
// STDOUT:str_eq("str", "ef"): 0
// STDOUT:str_eq("str", "str"): 1
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

