#include <scc/scc_bits.h>

#include <math.h>

#include <unity.h>

void test_scc_bits_is_power_of_2(void) {
    enum { TEST_SIZE = 320000 };

    for(int i = 1; i < TEST_SIZE; ++i) {
        float f = log2f(i);
        if((1 << (int)f) == i) {
            TEST_ASSERT_TRUE(scc_bits_is_power_of_2(i));
        }
        else {
            TEST_ASSERT_FALSE(scc_bits_is_power_of_2(i));
        }
    }
}
