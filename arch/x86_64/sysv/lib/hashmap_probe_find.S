#include "avx2_trampoline.h"
#include "hash_find_probe.h"

# Probe for value in hash map
#
# Params:
#   %rdi: Base address of hash map
#   %rsi: Address of hm_curr
#   %rdx: Key size
#   %rcx: Hash of hm_curr
#
# Return:
#   %rax: Index of slot, or -1 if not found
avx2_hashmap_probe_find:
.equ    mdoff,   0x18                       # Offset of hm_mdoff relative base address
.equ    cap,     0x28                       # Offset of hm_capacity relative base address
.equ    pairsz,  0x30                       # Offset of hm_pairsize relative base address
.equ    framesz, 0x58                       # Size of stack frame
.equ    vecsize, 0x20
#ifdef SCC_PERFEVTS
.equ    neqs,    0x40                       # Offset of ev_n_eqs in perf member, relative base
#endif

    popcntq %rdx, %rax                      # Check if element is a power of 2
    cmpq    $1, %rax
    je      .Lpwr2
    hash_find_probe 0, 1
.Lpwr2:
    hash_find_probe 1, 1

.globl scc_hashmap_impl_probe_find_avx2_trampoline
scc_hashmap_impl_probe_find_avx2_trampoline:
    avx2_trampoline avx2_hashmap_probe_find, scc_hashmap_impl_probe_find_swar
