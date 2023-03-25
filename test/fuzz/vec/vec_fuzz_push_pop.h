#ifndef VEC_FUZZ_PUSH_POP_H
#define VEC_FUZZ_PUSH_POP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool vec_fuzz_push_pop(uint32_t const *data, size_t size);

#endif /* VEC_FUZZ_PUSH_POP_H */
