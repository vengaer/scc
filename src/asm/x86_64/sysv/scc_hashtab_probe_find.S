#include "generic_hash_find_probe.S"

# Probe for value in hash table
#
# Params:
#   %rdi: Base address of hash table
#   %rsi: Address of ht_curr
#   %rdx: Element size
#   %rcx: Hash of ht_curr
#
# Return:
#   %rax: Index of slot, or -1 if element
#         is already present
.globl scc_hashtab_probe_find
scc_hashtab_probe_find:
.equ    mdoff,   0x10                       # Offset of ht_mdoff relative base address
.equ    cap,     0x20                       # Offset of ht_capacity relative base address
.equ    framesz, 0x58                       # Size of stack frame
.equ    vecsize, SCC_VECSIZE
#ifdef SCC_PERFEVTS
.equ    neqs,    0x30                       # Offset of ev_n_eqs in perf member, relative base
#endif

    popcntq %rdx, %rax                      # Check if element size is power of 2
    cmpq    $1, %rax
    je      .Lpwr2
    generic_hash_find_probe 0
.Lpwr2:
    generic_hash_find_probe 1
