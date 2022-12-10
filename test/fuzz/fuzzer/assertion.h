#ifndef ASSERTION_H
#define ASSERTION_H

#include <scc/pp_token.h>

#include <stdio.h>
#include <stdlib.h>

#define fuzz_chk_pick(_, x, ...) x
#define fuzz_chk(...) fuzz_chk_pick(__VA_ARGS__,0,)
#define fuzz_generic_probe() x,1
#define fuzz_probe_1_1() fuzz_generic_probe()

#define fuzz_match(token, match)                        \
    fuzz_chk(                                           \
        scc_pp_expand(                                  \
            scc_pp_cat_expand(                          \
                scc_pp_cat_expand(                      \
                    scc_pp_cat_expand(                  \
                        fuzz_probe_,                    \
                        token                           \
                    ),                                  \
                    _                                   \
                ),                                      \
                match                                   \
            )                                           \
        )()                                             \
    )

#define fuzz_count_sequence 16, 15, 14, 13,             \
                            12, 11, 10, 9,              \
                            8,  7,  6,  5,              \
                            4,  3,  2,  1,              \
                            0

#define fuzz_counter_pick(a0,  a1,  a2,  a3,            \
                          a4,  a5,  a6,  a7,            \
                          a8,  a9,  a10, a11,           \
                          a12, a13, a14, a15,           \
                          a16, ...) a16

#define fuzz_count_expand(...)                          \
    fuzz_counter_pick(__VA_ARGS__)
#define fuzz_count(...)                                 \
    fuzz_count_expand(__VA_ARGS__,fuzz_count_sequence)

#define fuzz_assert_cond_only_1(cond)                   \
    if(!(cond)) {                                       \
        (void)fprintf(stderr, "%s:%d: Assertion '%s' "  \
                        "failed\n", __FILE__,           \
                        __LINE__, #cond);               \
        abort();                                        \
    }

#define fuzz_assert_no_varargs_0(cond, fmt, ...)        \
    if(!(cond)) {                                       \
        (void)fprintf(stderr, "%s:%d: Assertion '%s' "  \
                        "failed: " fmt "\n",            \
                        __FILE__, __LINE__,             \
                        #cond, __VA_ARGS__);            \
        abort();                                        \
        }

#define fuzz_assert_no_varargs_1(cond, str)             \
    if(!(cond)) {                                       \
        (void)fprintf(stderr, "%s:%d: Assertion '%s' "  \
                        "failed: " str "\n",            \
                        __FILE__, __LINE__,             \
                        #cond);                         \
        abort();                                        \
    }

#define fuzz_assert_cond_only_0(cond, ...)              \
    scc_pp_cat_expand(                                  \
        fuzz_assert_no_varargs_,                        \
        fuzz_match(                                     \
            fuzz_count(__VA_ARGS__),                    \
            1                                           \
        )                                               \
    )(cond, __VA_ARGS__)


#define fuzz_assert(...)                                \
    scc_pp_cat_expand(                                  \
        fuzz_assert_cond_only_,                         \
        fuzz_match(                                     \
            fuzz_count(__VA_ARGS__),                    \
            1                                           \
        )                                               \
    )(__VA_ARGS__)

#endif /* ASSERTION_H */
