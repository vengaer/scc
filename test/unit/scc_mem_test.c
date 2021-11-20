#include <scc/scc_mem.h>

#include <unity.h>

void test_scc_container(void) {
    struct foo {
        unsigned dw;
        short wd;
        unsigned char b;
    } f = { 1, 2, 3 };

    unsigned char *c = &f.b;
    TEST_ASSERT_EQUAL_UINT32(1, scc_container(c, struct foo, b)->dw);
    TEST_ASSERT_EQUAL_INT16(2, scc_container(c, struct foo, b)->wd);
    TEST_ASSERT_EQUAL_UINT8(3, scc_container(c, struct foo, b)->b);

    unsigned *u = &f.dw;
    TEST_ASSERT_EQUAL_UINT32(1, scc_container(u, struct foo, dw)->dw);
    TEST_ASSERT_EQUAL_INT16(2, scc_container(u, struct foo, dw)->wd);
    TEST_ASSERT_EQUAL_UINT8(3, scc_container(u, struct foo, dw)->b);

    short *s = &f.wd;
    TEST_ASSERT_EQUAL_UINT32(1, scc_container(s, struct foo, wd)->dw);
    TEST_ASSERT_EQUAL_INT16(2, scc_container(s, struct foo, wd)->wd);
    TEST_ASSERT_EQUAL_UINT8(3, scc_container(s, struct foo, wd)->b);
}
