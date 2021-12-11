#ifndef SVEC_FUZZ_PUSH_POP_H
#define SVEC_FUZZ_PUSH_POP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool svec_fuzz_push_pop(uint32_t const *data, size_t size);

#endif /* SVEC_FUZZ_PUSH_POP_H */
