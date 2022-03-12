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
.macro isolssbl  r0, r1
    leal    -1(\r0), \r1
    notl    \r1
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
#   \pwr2: 1 if element size is a power of 2, 0 otherwise
#   \dupl: Label to jump to on identified duplicate
#
# Return:
#   -
.macro eqloop pwr2, dupl
0:
    leaq    (%rbx, %r14), %rdx              # Array index
    wrapq   %rbp, %rdx, %rcx                # Wrap index
    leaq    1(%rdx), %rax                   # Compensate for %r13 holding &ht_data[-1]

.if \pwr2
    movq    %r12, %rcx                      # Shift for offset
    shlq    %cl, %rax
.else
    mulq    %r12
.endif

    movq    %r13, %rsi                      # Address of value pending insertion
    leaq    (%r13, %rax), %rdi              # Address of potential matchi in table

    movq    (%rbp), %rax                    # Call eq
    call    *%rax

    ieqcq   %rbp                            # Increment eq call counter

    testl   %eax, %eax                      # Check return value
    jnz     \dupl

    leal    -1(%r15d), %r14d                # Clear rightmost set bit
    andl    %r14d, %r15d
    tzcntl  %r15d, %r14d                    # Vector offset of next match
    jnc     0b
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
#   pwr2: 1 if element size is a power of 2, 0 otherwise
#
# Return:
#   %rax: Index of slot, or -1 if element
#         is already present
.macro do_probe pwr2
.if \pwr2
    tzcntq  %rdx, %rdx                      # Element size power of 2, use bit index for efficient mult
.endif

    movq    %rcx, %rax                      # Compute slot
    wrapq   %rdi, %rax, %r8

    shrq    $57, %rcx                       # Broadcast high 7 bits of hash to ymm15
    orl     $0x80, %ecx                     # Set high bit for comparison with occupied slots
    vmovd   %ecx, %xmm0
    vpbroadcastb    %xmm0, %ymm15

    vpcmpeqb    %ymm14, %ymm14, %ymm14      # All ones
    vpxor   %ymm13, %ymm13, %ymm13          # All zeroes

9:
    movq    mdoff(%rdi), %r8                # Load ht_mdoff
    leaq    (%rax, %r8), %rcx               # Offset of metadata entry relative base

    vmovdqu (%rdi, %rcx), %ymm0             # Load ymmword's worth of metadata

    vpcmpeqb    %ymm0, %ymm15, %ymm1        # All ones for occupied with match
    vpmovmskb   %ymm1, %ecx                 # Exract bitmask

    vpcmpeqb    %ymm0, %ymm13, %ymm2        # All ones for probe end
    vpmovmskb   %ymm2, %r8d                 # Extract bitmask

    testl   %r8d, %r8d                      # Check for probe end
    jz      3f                              # No end, must probe again

    isolssbl    %r8d, %r9d                  # Isolate rightmost 1-bit
    subl    $1, %r8d                        # Compute mask
    andl    %r8d, %ecx                      # Mask out matches beyond probe end
    jz      1f                              # No match, guaranteed probe end in vector

    framesetup                              # Must call eq at least once

    tzcntl  %ecx, %r14d                     # Vector offset of first match

    eqloop  \pwr2, 2f

    framerst                                # No match, restore frame

1: # no match
    movq    $-1, %rax
    vzeroupper
    ret

2: # match
    leaq    (%rbx, %r14), %rax              # Element index
    wrapq   %rbp, %rax, %rcx                # Wrap
    framerst                                # Restore stack
    vzeroupper
    ret

3: # repeated probe
    testl   %ecx, %ecx                      # Check if match
    jz      6f

    framesetup                              # Prepare stack
    vmovdqu %ymm15, 0x30(%rsp)              # Preserve packed hash

4:
    tzcntl  %r15d, %r14d                    # Vector offset of match

    eqloop  \pwr2, 2b

    vmovdqu 0x20(%rsp), %ymm15              # Restore bcasted hash
    vpcmpeqb    %ymm14, %ymm14, %ymm14      # All ones
    vpxor   %ymm13, %ymm13, %ymm13          # All zeroes

    leaq    0x20(%rbx), %rbx                # Advance and wrap
    wrapq   %rbp, %rbx, %rcx

    movq    mdoff(%rbp), %r8                # Load ht_mdoff
    leaq    (%rbx, %r8), %rcx               # Offset of metadata entry relative base

    vmovdqu (%rbp, %rcx), %ymm0             # Load ymmword's worth of metadata

    vpcmpeqb    %ymm0, %ymm15, %ymm1        # All ones for occupied with match
    vpmovmskb   %ymm1, %r15d                # Exract bitmask

    vpcmpeqb    %ymm0, %ymm13, %ymm2        # All ones for probe end
    vpmovmskb   %ymm2, %r8d                 # Extract bitmask

    testl   %r8d, %r8d                      # Check for probe end
    jz      4b

    isolssbl    %r8d, %r9d                  # Isolate rightmost 1-bit
    subl    $1, %r8d                        # Compute mask
    andl    %r8d, %r15d                     # Mask out matches beyond probe end
    jz      5f                              # No match, guaranteed probe end in vector

    tzcntl  %r15d, %r14d                    # Vector offset of first match

    eqloop  \pwr2, 2b

5: # no match
    framerst                                # Restore frame
    movq    $-1, %rax
    vzeroupper
    ret

6: # no match, no eq calls
    leaq    0x20(%rax), %rax                # No match in vector, advance and wrap
    wrapq   %rdi, %rax, %rcx
    jmp  9b
.endm

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
#ifdef SCC_PERFEVTS
.equ    neqs,    0x28                       # Offset of ev_n_eqs in perf member, relative base
#endif

    popcntq %rdx, %rax                      # Check if element size is power of 2
    cmpq    $1, %rax
    je      .Lpwr2
    do_probe 0
.Lpwr2:
    do_probe 1
