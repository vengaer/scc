#include <scc/hash.h>
#include <scc/mem.h>
#include <scc/murmur64.h>

#include <string.h>

#include <unity.h>

struct aligned_string {
    uint64_t dummy;
    char str[512u];
};

struct unaligned_string {
    uint64_t dummy;
    unsigned char misalign;
    char str[512u];
};

void test_murmur3_128_64_vectors(void) {
    struct {
        char const *input;
        unsigned char const digest[16u];
    } vectors[] = {
        {
            .input = "halcyon",
            .digest = {
                0xbd, 0xee, 0x7b, 0x22, 0x9a, 0xd7, 0x48, 0xcd,
                0x88, 0x7f, 0x69, 0x3e, 0xe0, 0x59, 0x61, 0x32
            },
        },
        {
            .input = "perdition",
            .digest = {
                0xb3, 0xd0, 0x5d, 0x39, 0xeb, 0x19, 0xb8, 0xeb,
                0xab, 0x35, 0x6c, 0x2a, 0x42, 0x57, 0xec, 0x14
            },
        },
        {
            .input = "Atme ich nichts als Staub",
            .digest = {
                0x35, 0x39, 0xc0, 0x42, 0x97, 0x8, 0x8f, 0x9a,
                0x58, 0x33, 0x9f, 0xf1, 0xa2, 0xe, 0x4e, 0x6a
            },
        },
        {
            .input = "Und ich falle zuruck ins All",
            .digest = {
                0x5c, 0xcf, 0xd3, 0xdd, 0x4f, 0xbe, 0xaa, 0x5f,
                0x3c, 0x36, 0x58, 0x4d, 0xa8, 0x6f, 0xbc, 0xdf
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
                0x3b, 0xbb, 0xd2, 0x95, 0x07, 0xed, 0xf8, 0xa9,
                0x3c, 0x96, 0xba, 0xba, 0xdf, 0x52, 0xfc, 0x21
            },
        },
    };

    for (unsigned i = 0u; i < scc_arrsize(vectors); ++i) {
        struct aligned_string s = { 0 };
        TEST_ASSERT_EQUAL_UINT32(0u, (unsigned long)s.str & (scc_alignof(uint64_t) - 1u));
        strcpy(s.str, vectors[i].input);

        struct scc_digest128 digest;
        scc_murmur64_128(&digest, s.str, strlen(s.str), 0);
        TEST_ASSERT_EQUAL_INT32(0, memcmp(&digest, vectors[i].digest, sizeof(digest)));
    }
}

void test_murmur3_128_64_misaligned(void) {
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

        TEST_ASSERT_EQUAL_UINT32(0u, (unsigned long)a.str & (scc_alignof(uint64_t) - 1u));
        TEST_ASSERT_NOT_EQUAL_UINT32(0u, (unsigned long)u.str & (scc_alignof(uint64_t) - 1u));

        struct scc_digest128 aligned;
        struct scc_digest128 unaligned;

        scc_murmur64_128(&aligned, a.str, strlen(a.str), 0);
        scc_murmur64_128(&unaligned, u.str, strlen(a.str), 0);

        TEST_ASSERT_EQUAL_INT32(0, memcmp(&aligned, &unaligned, sizeof(aligned)));
    }
}
