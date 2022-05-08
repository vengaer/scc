#include <scc/scc_ringdeque.h>

#include <unity.h>

void test_scc_ringdeque_init(void) {
    scc_ringdeque(int) deque = scc_ringdeque_init(int);
    scc_ringdeque_free(deque);
}
