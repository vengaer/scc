#ifndef HASH_FIND_PROBE_H
#define HASH_FIND_PROBE_H

#include "isolssb.h"

# Wrap index of slot computation
#
# Sets ZF if result is 0
#
# Params:
#   \base: Base address of hash table
#   \r0:   Index to wrap
#   \r1:   Scratch register
#
# Return:
#   \r0: The value of \r0 & (base->ht_capacity - 1)
.macro wrapq    base, r0, r1
    movq    cap(\base), \r1
    subq    $1, \r1
    andq    \r1, \r0
.endm

# Increment equality cardinality. \base is the
# base address of the hashtab struct
#
# Params:
#   \base: Base address of hash table
#
# Return:
#   -
.macro ieqcq    base
#ifdef SCC_PERFEVTS
    addq $1, neqs(\base)                    # Increment eq call counter
#endif
.endm

# Prepare stack frame for eq calls
.macro framesetup
    subq    $framesz, %rsp                  # Make space and align rsp
    movq    %r15, 0x28(%rsp)                # Use non-scratch registers to preserve data across calls
    movl    %ecx, %r15d                     # Hash match mask

    movq    %r14, 0x20(%rsp)                # Scratch

    movq    %r13, 0x18(%rsp)
    movq    %rsi, %r13                      # Address of ht_curr

    movq    %r12, 0x10(%rsp)
    movq    %rdx, %r12                      # Element size

    movq    %rbx, 0x08(%rsp)
    movq    %rax, %rbx                      # Slot index

    movq    %rbp, (%rsp)
    movq    %rdi, %rbp                      # Base address
.endm

# Restore stack frame
.macro framerst
    movq    (%rsp), %rbp
    movq    0x08(%rsp), %rbx
    movq    0x10(%rsp), %r12
    movq    0x18(%rsp), %r13
    movq    0x20(%rsp), %r14
    movq    0x28(%rsp), %r15
    addq    $framesz, %rsp
.endm

# Main eq loop. Calls eq on each slot whose corresponding
# bit is set in %r15d.
#
# Prerequisites:
#   %rbx:  Slot index
#   %rbp:  Hashtab base address
#   %r12:  Element size (or log2(element size) if \pwr2 == 1)
#   %r13:  Address of ht_curr
#   %r14d: 0-based index of low bit in %r15d (i.e. tzcntl %r15d, %r14d)
#   %r15:  Hash match mask
#
# Params:
#   \pwr2:  1 if element size is a power of 2, 0 otherwise
#   \map:   1 if probing a map, 0 if probing a table
#   \label: Name of loop label
#   \dupl:  Label to jump to on identified duplicate
#
# Return:
#   -
.macro eqloop pwr2, map, label, dupl
    leaq    (%rbx, %r14), %rax              # Array index
    wrapq   %rbp, %rax, %rcx                # Wrap index

.if \map
    movq    pairsz(%rbp), %rdx              # Size of key-value pair
.else
    leaq    1(%rax), %rax                   # Compensate for %r13 holding &ht_data[-1]
.endif

.if \pwr2
    movq    %r12, %rcx                      # Shift for offset
    shlq    %cl, %rax
.else
    mulq    %r12
.endif

.if \map
    leaq    (%rax, %rdx), %rax              # Compensate for size of key-value pair
.endif

    movq    %r13, %rsi                      # Address of value pending insertion
    leaq    (%r13, %rax), %rdi              # Address of potential match in table

    movq    (%rbp), %rax                    # Call eq
    call    *%rax

    ieqcq   %rbp                            # Increment eq call counter

    testl   %eax, %eax                      # Check return value
    jnz     \dupl

    leal    -1(%r15d), %r14d                # Clear rightmost set bit
    andl    %r14d, %r15d
    tzcntl  %r15d, %r14d                    # Vector offset of next match
    jnc     \label
.endm

# Actual probing logic
#
# Prerequisites:
#   %rdi: Base address of hash table
#   %rsi: Address of ht_curr
#   %rdx: Element size
#   %rcx: Hash of ht_curr
#
# Params:
#   \pwr2: 1 if element size is a power of 2, 0 otherwise
#   \map:  1 if probing a map, 0 if probing a table
#
# Return:
#   %rax: Index of slot, or -1 if element
#         is already present
.macro hash_find_probe pwr2, map
.if \pwr2
    tzcntq  %rdx, %rdx                      # Element size power of 2, use bit index for efficient mult
.endif

    movq    %rcx, %rax                      # Compute slot

    movq    cap(%rdi), %r10                 # Wrap index
    leaq    -1(%r10), %r10
    andq    %r10, %rax

    shrq    $57, %rcx                       # Broadcast high 7 bits of hash to %ymm15
    orl     $0x80, %ecx                     # Set high bit for comparison with occupied slots
    vmovd   %ecx, %xmm0
    vpbroadcastb    %xmm0, %ymm15

    vpxor   %ymm14, %ymm14, %ymm14          # All zeroes

    movq    mdoff(%rdi), %r11               # Load ht_mdoff
    leaq    (%rax, %r11), %rcx              # Offset of metadata entry relative base

    vmovdqu (%rdi, %rcx), %ymm0             # Load ymmword's worth of metadata

    vpcmpeqb    %ymm0, %ymm15, %ymm1        # All ones for occupied with match
    vpmovmskb   %ymm1, %ecx                 # Extract bitmask

    vpcmpeqb    %ymm0, %ymm14, %ymm2        # All ones for probe end
    vpmovmskb   %ymm2, %r8d                 # Extract bitmask

    testl   %r8d, %r8d                      # Check for probe end
    jz      8f

    isolssbl    %r8d, %r9d                  # Isolate rightmost set bit
    leal    -1(%r9d), %r9d
    andl    %r9d, %ecx                      # Mask out matches beyond probe end
    jz      1f

    framesetup                              # Prepare frame for eq calls
    tzcntl  %r15d, %r14d                    # Vector offset of first match

0: # Match exists, probe end in vector
    eqloop  \pwr2, \map, 0b, 2f

    framerst

1: # No match
    movq    $-1, %rax
    vzeroupper
    retq

2: # Match found
    leaq    (%rbx, %r14), %rax              # Element index
    wrapq   %rbp, %rax, %rcx                # Wrap
    framerst
    vzeroupper
    retq

8: # No probe end
    leaq    -1(%rax), %r8                   # Compute index of last entry to probe
    andq    %r10, %r8                       # Wrap

    leaq    (%r11, %r8), %r9                # Offset of last entry relative base
    movzbl  (%rdi, %r9), %r11d              # Preserve metadata entry

    movb    $0, (%rdi, %r9)                 # Ensure probe end exists
    cmpq    $vecsize, %r8                   # Check if entry is mirrored
    jnb     6f

    leaq    1(%r9, %r10), %r10              # Offset of mirrored entry
    movb    $0, (%rdi, %r10)                # Clear mirrored entry

    testl   %ecx, %ecx                      # Check for match
    jnz     9f

7: # No probe end, no match
    leaq    0x20(%rax), %rax                # Advance and wrap
    wrapq   %rdi, %rax, %rcx

    movq    mdoff(%rdi), %r10               # Load ht_mdoff
    leaq    (%rax, %r10), %rcx              # Offset of metadata entry relative base

    vmovdqu (%rdi, %rcx), %ymm0             # Load ymmword's worth of metadata

    vpcmpeqb    %ymm0, %ymm15, %ymm1        # All ones for occupied with match
    vpmovmskb   %ymm1, %ecx                 # Extract bitmask

    vpcmpeqb    %ymm0, %ymm14, %ymm2        # All ones for probe end
    vpmovmskb   %ymm2, %r8d                 # Extract bitmask

    testl   %r8d, %r8d                      # Check for probe end
    jz      6f

    isolssbl    %r8d, %r10d                 # Isolate rightmost set bit
    leal    -1(%r10d), %r10d
    andl    %r10d, %ecx                     # Mask out matches beyond probe end
    jnz     5f

    movq    $-1, %rax                       # No match

    movb    %r11b, (%rdi, %r9)              # Restore metadata entry

    movq    mdoff(%rdi), %rsi               # Offset of metadata

    leaq    vecsize(%rsi), %rdx             # Check if entry is duplicated in guard
    cmpq    %rdx, %r9
    jnb     0f

    movq    cap(%rdi), %rsi                 # Capacity
    leaq    (%r9, %rsi), %rcx               # Offset of duplicate in guard

    movb    %r11b, (%rdi, %rcx)
0:
    vzeroupper
    retq

6: # No probe end
    testl   %ecx, %ecx                      # Check for match
    jz      7b
9: # No probe end, match exists
    framesetup                              # Must call eq

    shlq    $56, %r11                       # Store entry in high byte of offset
    orq     %r11, %r9
    movq    %r9, 0x30(%rsp)                 # Write to stack

    vmovdqu %ymm15, 0x38(%rsp)              # Preserve broadcasted hash

    tzcntl  %r15d, %r14d                    # Vector index of first match

0: # No probe end, match exists
    eqloop  \pwr2, \map, 0b, 4f

    vmovdqu 0x38(%rsp), %ymm15              # Restore hash
    vpxor   %ymm14, %ymm14, %ymm14          # All zeroes

1:
    leaq    0x20(%rbx), %rbx                # Advance
    wrapq   %rbp, %rbx, %rcx

    movq    mdoff(%rbp), %rdi               # Load ht_mdoff
    leaq    (%rbx, %rdi), %rcx              # Offset of metadata entry relative base

    vmovdqu (%rbp, %rcx), %ymm0             # Load ymmword's worth of metadata

    vpcmpeqb    %ymm0, %ymm15, %ymm1        # All ones for occupied with match
    vpmovmskb   %ymm1, %r15d                # Extract bitmask

    vpcmpeqb    %ymm0, %ymm14, %ymm2        # All ones for probe end
    vpmovmskb   %ymm2, %r8d                 # Extract bitmask

    testl   %r8d, %r8d                      # Check for probe end
    jnz     3f

    tzcntl  %r15d, %r14d                    # Vector index of match
    jnc     0b

    jmp     1b

3: # Probe end in vector
    isolssbl    %r8d, %r9d                  # Isolate rightmost set bit
    leal    -1(%r9d), %r9d
    andl    %r9d, %r15d                     # Mask out matches beyond probe end
    jz      3f

    tzcntl  %r15d, %r14d                    # Vector offset of match
    jmp     0f

5: # Probe end and match in vector
    framesetup                              # Must call eq, prepare stack

    shlq    $56, %r11                       # Store entry in high byte of offset
    orq     %r11, %r9
    movq    %r9, 0x30(%rsp)                 # Write to stack

    tzcntl  %r15d, %r14d                    # Vector index of first match
0:
    eqloop  \pwr2, \map, 0b, 4f

3:
    movq    0x30(%rsp), %rcx                # Restore metadata

    movq    %rcx, %rdx                      # Extract entry
    shrq    $56, %rdx

    movq    $0xffffffffffffff, %rdi         # Mask
    andq    %rdi, %rcx                      # Offset relative base

    movb    %dl, (%rbp, %rcx)               # Restore metadata offset

    movq    mdoff(%rbp), %rbx               # Offset of metadata relative base
    leaq    vecsize(%rbx), %rbx

    cmpq    %rbx, %rcx                      # Check if entry is duplicated in guard
    jnb     0f

    movq    cap(%rbp), %rsi                 # Capacity
    leaq    (%rcx, %rsi), %rdi

    movb    %dl, (%rbp, %rdi)               # Restore guard entry
0:
    framerst                                # Restore frame
    movq    $-1, %rax                       # No match
    vzeroupper
    retq

4:
    leaq    (%rbx, %r14), %rax              # Element index
    wrapq   %rbp, %rax, %rcx                # Wrap

    movq    0x30(%rsp), %rcx                # Restore metadata

    movq    %rcx, %rdx                      # Extract entry
    shrq    $56, %rdx

    movq    $0xffffffffffffff, %rdi         # Mask
    andq    %rdi, %rcx                      # Offset relative base

    movb    %dl, (%rbp, %rcx)               # Restore metadata offset

    movq    mdoff(%rbp), %rbx               # Offset of metadata relative base
    leaq    vecsize(%rbx), %rbx

    cmpq    %rbx, %rcx                      # Check if entry is duplicated in guard
    jnb     4f

    movq    cap(%rbp), %rsi                 # Capacity
    leaq    (%rcx, %rsi), %rdi

    movb    %dl, (%rbp, %rdi)               # Restore guard entry
4:
    framerst                                # Restore stack
    vzeroupper
    retq
.endm

#endif /* HASH_FIND_PROBE_H */
