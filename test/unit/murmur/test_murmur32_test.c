#include <scc/hash.h>
#include <scc/mem.h>
#include <scc/murmur32.h>

#include <string.h>

#include <unity.h>

struct aligned_string {
    uint32_t dummy;
    char str[512u];
};

struct unaligned_string {
    uint32_t dummy;
    unsigned char misalign;
    char str[512u];
};

void test_murmur3_128_32_vectors(void) {
    struct {
        char const *input;
        unsigned char const digest[16u];
    } vectors[] = {
        {
            .input = "halcyon",
            .digest = {
                0xa8, 0x86, 0x45, 0x58, 0xd6, 0xac, 0x4d, 0x6a,
                0xb6, 0x67, 0xf0, 0x64, 0xb6, 0x67, 0xf0, 0x64
            },
        },
        {
            .input = "perdition",
            .digest = {
                0xc3, 0x5e, 0xe9, 0xdd, 0x88, 0x78, 0x12, 0x6d,
                0x05, 0x2d, 0x52, 0x31, 0x13, 0xa7, 0x7b, 0x85
            },
        },
        {
            .input = "Atme ich nichts als Staub",
            .digest = {
                0x87, 0x1e, 0x8, 0x2b, 0xc7, 0xfc, 0xc0, 0x3d,
                0x16, 0x45, 0x1f, 0x91, 0xb8, 0xb5, 0x1a, 0xc3
            },
        },
        {
            .input = "Und ich falle zuruck ins All",
            .digest = {
                0x6f, 0x6f, 0x48, 0x21, 0xf3, 0xf4, 0x38, 0x9c,
                0x2f, 0xac, 0x8d, 0xf9, 0xfb, 0x95, 0x4d, 0x2b
            },
        },
        {
            .input = "You're an idiot"
                     "You embody every bit of it"
                     "Even set a new precedent"
                     "Empirical"
                     ""
                     "Concrete evidence"
                     "Minimal due diligence"
                     "We've concluded what is obvious"
                     "You're a bunghole"
                     ""
                     "Concrete evidence"
                     "Minimal due diligence"
                     "We've concluded what is obvious"
                     "Bloody drongo"
                     ""
                     "Your voice induces migraines"
                     "Your presence party-poops the entire room",
            .digest = {
                0x38, 0x25, 0xce, 0x42, 0x32, 0xc1, 0x9e, 0xab,
                0x03, 0xce, 0x39, 0xa3, 0xe3, 0x1c, 0xf4, 0xc9
            }
        }
    };

    for (unsigned i = 0u; i < scc_arrsize(vectors); ++i) {
        struct aligned_string s = { 0 };
        TEST_ASSERT_EQUAL_UINT32(0u, (unsigned long)s.str & (scc_alignof(uint32_t) - 1u));
        strcpy(s.str, vectors[i].input);

        struct scc_digest128 digest;
        scc_murmur32_128(&digest, s.str, strlen(s.str), 0);
        TEST_ASSERT_EQUAL_INT32(0, memcmp(&digest, vectors[i].digest, sizeof(digest)));
    }
}

void test_murmur3_128_32_misaligned(void) {
    char const *inputs[] = {
        "Ae bac to bi ri to",
        "Jul eis. Tae kih lei ae",
        "A confused feeling in the memory of a few forgotten men",
        "In the cleverest flesh that comes breathing",
        "Of dream and this needle in his eyes",
    };

    for (unsigned i = 0u; i < scc_arrsize(inputs); ++i) {
        struct aligned_string a = { 0 };
        struct unaligned_string u = { 0 };

        strcpy(a.str, inputs[i]);
        strcpy(u.str, inputs[i]);

        TEST_ASSERT_EQUAL_UINT32(0u, (unsigned long)a.str & (scc_alignof(uint32_t) - 1u));
        TEST_ASSERT_NOT_EQUAL_UINT32(0u, (unsigned long)u.str & (scc_alignof(uint32_t) - 1u));

        struct scc_digest128 aligned;
        struct scc_digest128 unaligned;

        scc_murmur32_128(&aligned, a.str, strlen(a.str), 0);
        scc_murmur32_128(&unaligned, u.str, strlen(a.str), 0);

        TEST_ASSERT_EQUAL_INT32(0, memcmp(&aligned, &unaligned, sizeof(aligned)));
    }
}
