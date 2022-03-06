# Isolate rightmost 1-bit in non-zero 32-bit
# register \r0. \r1 is used as scratch
#
# Sets ZF if the result is 0
#
# Params:
#   \r0: Register to isolate the rightmost set bit in
#   \r1: Scratch
#
# Return:
#   \r0: The result of \r0 & ~(\r0 - 1)
.macro isolsbl  r0, r1
    leal    -1(\r0), \r1
    notl    \r1
    andl    \r1, \r0
.endm

# Clear rightmost 1-bit in 32-bit register
#
# Sets ZF if the result is 0
#
# Params:
#   \r0: Register to clear the rightmost set bit in
#   \r1: Scratch
#
# Return:
#   \r0: The result of \r0 & (\r0 - 1)
.macro clrlsbl  r0, r1
    leal    -1(\r0), \r1
    andl    \r1, \r0
.endm

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
    movl    %r8d, %r15d                     # Hash match mask

    movq    %r14, 0x20(%rsp)
    movl    %ecx, %r14d                     # Vacant mask

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
#   %r9d: 0-based index of low bit in %r15d (i.e. tzcntl %r15d, %r9d)
#   %rbx: Slot index
#   %rbp: Hashtab base address
#   %r12: Element size (or log2(element size) if \pwr2 == 1)
#   %r13: Address of ht_curr
#   %r15: Hash match mask
#
# Params:
#   \pwr2: 1 if element size is a power of 2, 0 otherwise
#   \dupl: Label to jump to on identified duplicate
#
# Return:
#   -
.macro eqloop pwr2, dupl
0:
    leaq    (%rbx, %r9), %rdx               # Array index
    wrapq   %rbp, %rdx, %rcx                # Wrap index
    leaq    1(%rdx), %rax                   # Compensate for %r13 holding &ht_data[-1]

.if \pwr2
    movq    %r12, %rcx                      # Shift for offset
    shlq    %cl, %rax
.else
    mul     %r12                            # Multiply for offset
.endif

    movq    %r13, %rsi                      # Address of value pending insertion
    leaq    (%r13, %rax), %rdi              # Address of potential match in table

    movq    (%rbp), %rax                    # Call eq
    call    *%rax

    ieqcq   %rbp                            # Increment eq call counter

    testl   %eax, %eax                      # Check return value
    jnz     \dupl

    clrlsbl %r15d, %r8d                     # Clear rightmost set bit
    tzcntl  %r15d, %r9d                     # Vector index of next matching slot
    jnc     0b                              # Loop if more matches
.endm

# The actual probing logic. The capacity of the hash table
# is not considered at iteration - there must always be at
# least one vacant slot.
#
# Prerequisites:
#   %rdi: Base address of hash table
#   %rsi: Address of ht_curr
#   %rdx: Element size
#   %rcx: Hash of ht_curr
#
# Params:
#   pwr2: 1 if element size is a power of 2, 0 otherwise
#
# Return:
#   %rax: Index of slot, or -1 if element
#         is already present
.macro do_probe pwr2
.equ    mdoff,   0x10                       # Offset of ht_mdoff relative base address
.equ    cap,     0x20                       # Offset of ht_capacity relative base address
.equ    framesz, 0x58                       # Size of stack frame
#ifdef SCC_PERFEVTS
.equ    neqs,    0x28                       # Offset of ev_n_eqs in perf member, relative base
#endif

.if \pwr2
    tzcntq  %rdx, %rdx                      # Element size power of 2, use bit index for efficient mult
.endif

    movq    %rcx, %rax                      # Compute slot
    wrapq   %rdi, %rax, %r8

    shrq    $57, %rcx                       # Broadcast high 7 bits of hash to ymm15
    orl     $0x80, %ecx                     # Set MSB for comparison with occupied slots
    vmovd   %ecx, %xmm0
    vpbroadcastb    %xmm0, %ymm15

    vpcmpeqb    %ymm14, %ymm14, %ymm14      # All ones
    vpxor   %ymm13, %ymm13, %ymm13          # All zeroes

9:
    movq    mdoff(%rdi), %r8                # Load ht_mdoff
    leaq    (%rax, %r8), %rcx               # Offset of metadata entry relative base

    vmovdqu (%rdi, %rcx), %ymm0             # Load ymmword's worth of metadata

    vpxor   %ymm0, %ymm14, %ymm1            # MSB 1 for vacant slots
    vpmovmskb   %ymm1, %ecx                 # Extract bitmask

    vpcmpeqb    %ymm0, %ymm15, %ymm2        # All ones for occupied with matching hash
    vpmovmskb   %ymm2, %r8d                 # Extract bitmask

    vpcmpeqb    %ymm0, %ymm13, %ymm3        # All ones for probe end
    vpmovmskb   %ymm3, %r9d                 # Extract bitmask

    isolsbl  %r9d, %r10d                    # Isolate rightmost set bit
    subl    $1, %r9d                        # Compute hash match mask
    andl    %r9d, %r8d                      # Mask out matches beyond probe end
    jz      1f                              # ZF set if no match

    tzcntl  %r8d, %r9d                      # Vector index of next matching slot

    framesetup                              # Must call eq at least once

    testl   %ecx, %ecx                      # Check for at least one vacant in next 32 slots
    jz      3f                              # No vacant, must probe again

    eqloop  \pwr2, 2f                       # Do eq loop, at most 31 iters

                                            # No match found
    tzcntl  %r14d, %ecx                     # Vector offset of first vacant, at least one
    leaq    (%rbx, %rcx), %rax              # Compute index

    wrapq   %rbp, %rax, %rcx                # Wrap for return
    framerst                                # Restore stack
    vzeroupper
    ret

1:  # no matching hash in vector
    testl   %ecx, %ecx                      # Check for at least one vacant
    jnz     4f

    leaq    0x20(%rax), %rax                # Advance and wrap offset
    wrapq   %rdi, %rax, %rcx
    jmp     9b

4:
    tzcntl  %ecx, %edx                      # Vector offset of first vacant
    leaq    (%rax, %rdx), %rax              # Compute and wrap index
    wrapq   %rdi, %rax, %rcx

    vzeroupper
    ret

2:  # duplicate found
    framerst                                # Restore stack
    movq    $-1, %rax                       # Return value
    vzeroupper
    ret

3:  # probing required
    vmovdqu %ymm15, 0x30(%rsp)              # Preserve bcasted hash across eq calls

    eqloop  \pwr2, 2b                       # Do eq loop

                                            # No match found, probe again
    vmovdqu 0x30(%rsp), %ymm15              # Restore bcasted hash
    vpcmpeqb    %ymm14, %ymm14, %ymm14      # All ones
    vpxor   %ymm13, %ymm13, %ymm13          # All zeroes

    leaq    0x20(%rbx), %rbx                # Advance and wrapindex
    wrapq   %rbp, %rbx, %rcx

    movq    mdoff(%rbp), %r8                # Load ht_mdoff
    leaq    (%rbx, %r8), %rcx               # Offset of metadata entry relative base

    vmovdqu (%rbp, %rcx), %ymm0

    vpxor   %ymm0, %ymm14, %ymm1            # MSB 1 for vacant slots
    vpmovmskb   %ymm1, %r14d                # Extract bitmask

    vpcmpeqb    %ymm0, %ymm15, %ymm2        # All ones for occupied with matching hash
    vpmovmskb   %ymm2, %r15d                # Extract bitmask

    vpcmpeqb    %ymm0, %ymm13, %ymm3        # All ones for probe end
    vpmovmskb   %ymm3, %r9d                 # Extract bitmask

    isolsbl  %r9d, %r10d                    # Isolate rightmost set bit
    subl    $1, %r9d                        # Compute hash match mask
    andl    %r9d, %r15d                     # Mask out matches beyond probe end
    jz      1f                              # ZF set if no match

    tzcntl  %r15d, %r9d                     # Vector index of next matching slot

    testl   %r14d, %r14d                    # Check for at least one vacant in next 32 slots
    jz      3b                              # No vacant, repeat

    eqloop  \pwr2, 2b                       # Do eq loop

                                            # No match found
    tzcntl  %r14d, %ecx                     # Vector offset of first vacant, at least one
    leaq    (%rbx, %rcx), %rax              # Compute index

    wrapq   %rbp, %rax, %rcx                # Wrap for return
    framerst                                # Restore stack
    vzeroupper
    ret

1:  # no matching hash in vector
    testl   %r14d, %r14d
    jz      5f

    leaq    0x20(%rbx), %rbx                # Adavance and wrap offset
    wrapq   %rdi, %rbx, %rcx

    jmp     3b

5:
    tzcntl  %r14d, %edx                     # Vector offset of first vacant
    leaq    (%rbx, %rdx), %rax              # Compute and wrap index
    wrapq   %rbp, %rax, %rcx
    framerst                                # Reset stack frame
    vzeroupper
    ret

.endm

# Probe for empty slot to insert in. The hash table is
# assumed to have at least one vacant slot.
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
.globl scc_hashtab_probe_insert
scc_hashtab_probe_insert:
    popcntq %rdx, %rax                      # Check if element size is power of 2
    cmpq    $1, %rax
    je      .Lpwr2
    do_probe 0
.Lpwr2:
    do_probe 1
