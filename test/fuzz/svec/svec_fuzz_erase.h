#ifndef SVEC_FUZZ_ERASE_H
#define SVEC_FUZZ_ERASE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool svec_fuzz_erase(uint32_t const *data, size_t size);

#endif /* SVEC_FUZZ_ERASE_H */
