#include "generic_hash_find_probe.S"

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
.globl scc_hashmap_probe_find
scc_hashmap_probe_find:
.equ    mdoff,   0x18                       # Offset of hm_mdoff relative base address
.equ    cap,     0x28                       # Offset of hm_capacity relative base address
.equ    pairsz,  0x30                       # Offset of hm_pairsize relative base address
.equ    framesz, 0x58                       # Size of stack frame
.equ    vecsize, SCC_VECSIZE
#ifdef SCC_PERFEVTS
.equ    neqs,    0x40                       # Offset of ev_n_eqs in perf member, relative base
#endif

    popcntq %rdx, %rax                      # Check if element is a power of 2
    cmpq    $1, %rax
    je      .Lpwr2
    generic_hash_find_probe 0, 1
.Lpwr2:
    generic_hash_find_probe 1, 1
