#include "avx2_trampoline.h"
#include "hash_find_probe.h"

    .section .note.GNU-stack, "", @progbits
    .section .text

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
#         is not found
avx2_hashtab_find_probe:
.equ    mdoff,   0x10                       # Offset of ht_mdoff relative base address
.equ    cap,     0x20                       # Offset of ht_capacity relative base address
.equ    framesz, 0x58                       # Size of stack frame
.equ    vecsize, 0x20
#ifdef SCC_PERFEVTS
.equ    neqs,    0x30                       # Offset of ev_n_eqs in perf member, relative base
#endif

    popcntq %rdx, %rax                      # Check if element size is power of 2
    cmpq    $1, %rax
    je      .Lpwr2
    hash_find_probe 0, 0
.Lpwr2:
    hash_find_probe 1, 0

.globl scc_hashtab_impl_probe_find_avx2_trampoline
scc_hashtab_impl_probe_find_avx2_trampoline:
    avx2_trampoline avx2_hashtab_find_probe, scc_hashtab_impl_probe_find_swar
