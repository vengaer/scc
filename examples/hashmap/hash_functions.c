/* Example hashmap hash functions */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
 * 64-bit FNV-1a
 */
static uint_fast64_t fnv1a_64(void const *data, size_t size) {
    uint_fast64_t hash = UINT64_C(0xcbf29ce484222325);
    unsigned char const *dptr = data;
    for(size_t i = 0; i < size; ++i) {
        hash ^= dptr[i];
        hash *= UINT64_C(0x100000001b3);
    }
    return hash;
}

/*
 * 64-bit FNV-1
 */
static uint_fast64_t fnv1_64(void const *data, size_t size) {
    uint_fast64_t hash = UINT64_C(0xcbf29ce484222325);
    unsigned char const *dptr = data;
    for(size_t i = 0; i < size; ++i) {
        hash *= UINT64_C(0x100000001b3);
        hash ^= dptr[i];
    }
    return hash;

}

int main(void) {
    char const *str = "hash me";
    printf("fnv1a_64(\"%s\", strlen(\"%s\")): %" PRIxFAST64 "\n",
        str, str, fnv1a_64(str, strlen(str)));

    printf("fnv1_64(\"%s\", strlen(\"%s\")): %" PRIxFAST64 "\n",
        str, str, fnv1_64(str, strlen(str)));
}

/* ============= OUTPUT =============== */
// STDOUT:fnv1a_64("hash me", strlen("hash me")): 1ad66d8708e9833d
// STDOUT:fnv1_64("hash me", strlen("hash me")): 8ec4fa775a2b44d5
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
