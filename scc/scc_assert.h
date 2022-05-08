#ifndef SCC_ASSERT_H
#define SCC_ASSERT_H

#include "scc_pp_token.h"

#define scc_static_assert(...)                      \
    scc_pp_cat_expand(                              \
        scc_static_assert_cond_only_,               \
        scc_assert_match(                           \
            scc_assert_count(__VA_ARGS__),          \
            1                                       \
        )                                           \
    )(__VA_ARGS__)

#define scc_assert_match(token, match)              \
    scc_assert_chk(                                 \
        scc_pp_expand(                              \
            scc_pp_cat_expand(                      \
                scc_pp_cat_expand(                  \
                    scc_pp_cat_expand(              \
                        scc_assert_probe_,          \
                        token                       \
                    ),                              \
                    _                               \
                ),                                  \
                match                               \
            )                                       \
        )()                                         \
    )

#define scc_assert_count_sequence 2, 1, 0
#define scc_assert_counter_pick(a0, a1, a2, ...) a2
#define scc_assert_count_expand(...) scc_assert_counter_pick(__VA_ARGS__)
#define scc_assert_count(...) scc_assert_count_expand(__VA_ARGS__,scc_assert_count_sequence)

#define scc_assert_chk_pick(_, x, ...) x
#define scc_assert_chk(...) scc_assert_chk_pick(__VA_ARGS__,0,)
#define scc_assert_generic_probe() x,1
#define scc_assert_probe_1_1() scc_assert_generic_probe()

#define scc_static_assert_cond_only_1(cond)         \
    extern unsigned char scc_pp_cat_expand(scc_static_assert_,__LINE__)[(!!(cond) << 1) - 1]

#define scc_static_assert_cond_only_0(cond, msg)    \
    scc_static_assert_cond_only_1(cond)

#endif /* SCC_ASSERT_H */
