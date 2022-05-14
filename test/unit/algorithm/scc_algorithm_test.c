#include <scc/scc_algorithm.h>
#include <scc/scc_mem.h>

#include <stddef.h>

#include <unity.h>

static int compare(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

static size_t stupid_lower_bound(int val, int const *data, size_t size) {
    size_t i = 0u;
    while(i < size && data[i] < val) {
        ++i;
    }
    return i;
}

void test_scc_algo_lower_bound_0(void) {
    int data[] = { 1, 2, 5, 6, 21, 41, 52, 54, 63, 66, 83, 90 };

    int stupid;
    int bins;
    for(int i = 0; i < 100; ++i) {
        stupid = stupid_lower_bound(i, data, scc_arrsize(data));
        bins = scc_algo_lower_bound(&i, data, scc_arrsize(data), sizeof(i), compare);
        TEST_ASSERT_EQUAL_UINT64(stupid, bins);
    }
}

void test_scc_algo_lower_bound_1(void) {
    int data[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 23200 };

    int stupid;
    int bins;
    for(int i = 0; i < 232002; ++i) {
        stupid = stupid_lower_bound(i, data, scc_arrsize(data));
        bins = scc_algo_lower_bound(&i, data, scc_arrsize(data), sizeof(i), compare);
        TEST_ASSERT_EQUAL_UINT64(stupid, bins);
    }
}
