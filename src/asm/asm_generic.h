#ifndef ASM_GENERIC_H
#define ASM_GENERIC_H

#ifdef SCC_SIMD_ENABLED

#ifdef SCC_SIMD_ISA_AVX2
#include "x86_64/sysv/asm.h"
#endif

#else
#include "../simdemu/simd_swemu.h"
#endif

#endif /* ASM_GENERIC_H */
