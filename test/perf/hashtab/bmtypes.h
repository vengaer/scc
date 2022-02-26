#ifndef BMTYPES_H
#define BMTYPES_H

#ifndef SCC_BM_TYPE
#error SCC_BM_TYPE not defined
#endif

typedef SCC_BM_TYPE bm_type;

#ifdef __cplusplus
#include <type_traits>

static_assert(std::is_arithmetic_v<bm_type>);

#define ptrtype(ptr)    \
    std::remove_reference_t<decltype(*(ptr))>

#endif

#endif /* BMTYPES_H */
