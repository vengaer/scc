#include "avx2_trampoline.h"
#include "hash_insert_probe.h"

# Probe for empty slot to insert in. The hash map is
# assumed to have at least two vacant slots.
#
# Params:
#   %rdi: Base address of hash map
#   %rsi: Address of hm_curr
#   %rdx: Key size
#   %rcx: Hash of hm_curr->hp_key
#
# Return:
#   %rax: Index of slot
avx2_hashmap_probe_insert:
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
    hash_insert_probe 0, 1
.Lpwr2:
    hash_insert_probe 1, 1

.globl scc_hashmap_impl_probe_insert_avx2_trampoline
scc_hashmap_impl_probe_insert_avx2_trampoline:
    avx2_trampoline avx2_hashmap_probe_insert, scc_hashmap_impl_probe_insert_sw
