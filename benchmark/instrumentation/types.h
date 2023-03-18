#ifndef TYPES_H
#define TYPES_H

#ifndef SCC_BM_TYPE
#define SCC_BM_TYPE not defined
#endif

typedef SCC_BM_TYPE bm_type;

#ifdef __cplusplus

#include <type_traits>

static_assert(std::is_arithmetic_v<bm_type>);

#endif

#endif /* TYPES_H */
