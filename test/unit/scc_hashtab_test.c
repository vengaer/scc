#include <scc/scc_hashtab.h>
#include <scc/scc_mem.h>

#include <stdbool.h>

#include <unity.h>

static bool eq(void const *left, void const *right) {
    return *(int const *)left == *(int const *)right;
}

static unsigned long long ident(void const *data, size_t size) {
    switch(size) {
        case sizeof(unsigned char):
            return *(unsigned char const *)data;
        case sizeof(unsigned short):
            return *(unsigned short const *)data;
        case sizeof(unsigned):
            return *(unsigned const *)data;
        case sizeof(unsigned long long):
            return *(unsigned long long const *)data;
        default:
            TEST_ASSERT_EQUAL_UINT64(0ull, size);
    }
    return 0ull;
}

static struct scc_hashtab *tabbase(void *tab) {
    struct scc_hashtab *base = scc_container(
        tab - ((unsigned char *)tab)[-1] - sizeof(base->ht_fwoff),
        struct scc_hashtab,
        ht_fwoff
    );
    return base;
}

void test_scc_hashtab_bkoff(void) {
    scc_hashtab(int) hashtab = scc_hashtab_init(int, eq);
    struct scc_hashtab *base = tabbase(hashtab);
    /* Probably UB... */
    TEST_ASSERT_EQUAL_PTR(base->ht_eq, eq);
    scc_hashtab_free(hashtab);
}

void test_scc_hashtab_fwoff(void) {
    scc_hashtab(int) hashtab = scc_hashtab_init(int, eq);
    struct scc_hashtab *base = tabbase(hashtab);
    size_t const off = base->ht_fwoff + offsetof(struct scc_hashtab, ht_fwoff) + sizeof(base->ht_fwoff);
    int *fwp = (void *)((unsigned char *)base + off);
    TEST_ASSERT_EQUAL_PTR(hashtab, fwp);
    scc_hashtab_free(hashtab);
}

void test_scc_hashtab_explicit_hash(void) {
    scc_hashtab(int) hashtab = scc_hashtab_with_hash(int, eq, ident);
    struct scc_hashtab *base = tabbase(hashtab);
    /* UB */
    TEST_ASSERT_EQUAL_PTR(base->ht_hash, ident);
    scc_hashtab_free(hashtab);
}

void test_scc_hashtab_guard_initialized(void) {
    scc_hashtab(int) hashtab = scc_hashtab_init(int, eq);
    struct scc_hashtab *base = tabbase(hashtab);
    scc_hashtab_metatype *guard =
        (void *)((unsigned char *)base + base->ht_mdoff + base->ht_capacity * sizeof(*guard));

    for(unsigned i = 0u; i < scc_hashtab_impl_guardsz(); ++i) {
        TEST_ASSERT_EQUAL_UINT16(0x4000u, guard[i]);
    }

    scc_hashtab_free(hashtab);
}
