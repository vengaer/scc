#include <scc/scc_hashtab.h>
#include <scc/scc_mem.h>

#include <stdbool.h>

#include <unity.h>

static bool eq(void const *left, void const *right) {
    return *(int const *)left == *(int const *)right;
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
