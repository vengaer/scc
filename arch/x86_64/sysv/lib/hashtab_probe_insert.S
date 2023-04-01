#include "avx2_trampoline.h"
#include "hash_insert_probe.h"

# Probe for empty slot to insert in. The hash table is
# assumed to have at least two vacant slots.
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
avx2_hashtab_probe_insert:
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
    hash_insert_probe 0, 0
.Lpwr2:
    hash_insert_probe 1, 0

.globl scc_hashtab_impl_probe_insert_avx2_trampoline
scc_hashtab_impl_probe_insert_avx2_trampoline:
    avx2_trampoline avx2_hashtab_probe_insert, scc_hashtab_impl_probe_insert_swar
