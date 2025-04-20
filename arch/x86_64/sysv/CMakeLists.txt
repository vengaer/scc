set (SCC_VECSIZE 32 CACHE STRING "Vector size")
set (SCC_SIMD_ISA avx2 CACHE STRING "SIMD instruction set architecture")

set (SCC_SIMD_SUPPORTED ON CACHE BOOL "SIMD support")

file (GLOB ASM_SRC lib/*.S)

set (SCC_ASM_SRC ${ASM_SRC} CACHE STRING "")
