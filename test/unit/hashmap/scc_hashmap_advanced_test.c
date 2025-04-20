#include <scc/hash.h>
#include <scc/hashmap.h>

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <unity.h>

static bool streq(void const *l, void const *r) {
    return !strcmp(*(void *const *)l, *(void *const *)r);
}

static scc_hash_type strhash(void const *data, size_t size) {
    (void)size;
    return scc_hash_fnv1a(*(void *const *)data, strlen(*(void *const *)data));
}

void test_scc_hashmap_find_string(void) {
    enum { BUFSIZE = 32 };
    typedef struct {
        char buffer[BUFSIZE];
    } strtype;
    scc_hashmap(char const *, strtype) map =
        scc_hashmap_with_hash(char const *, strtype, streq, strhash);

    TEST_ASSERT_TRUE(scc_hashmap_insert(&map, "first key", (strtype){ "first value" }));
    TEST_ASSERT_TRUE(scc_hashmap_insert(&map, "second key", (strtype){ "second value" }));

    strtype *str = scc_hashmap_find(map, "first key");
    TEST_ASSERT_TRUE(str);
    TEST_ASSERT_EQUAL_STRING("first value", str->buffer);

    str = scc_hashmap_find(map, "second key");

    TEST_ASSERT_TRUE(str);
    TEST_ASSERT_EQUAL_STRING("second value", str->buffer);

    scc_hashmap_free(map);
}

void test_scc_hashmap_string_modification(void) {
    enum { BUFSIZE = 32 };
    typedef struct {
        char buffer[BUFSIZE];
    } strtype;
    scc_hashmap(char const *, strtype) map =
        scc_hashmap_with_hash(char const *, strtype, streq, strhash);

    TEST_ASSERT_TRUE(scc_hashmap_insert(&map, "key", (strtype){ "value" }));

    strtype *str = scc_hashmap_find(map, "key");
    TEST_ASSERT_TRUE(str);
    strcpy(str->buffer, "modified value");

    str = scc_hashmap_find(map, "key");
    TEST_ASSERT_TRUE(str);
    TEST_ASSERT_EQUAL_STRING("modified value", str->buffer);

    scc_hashmap_free(map);
}
