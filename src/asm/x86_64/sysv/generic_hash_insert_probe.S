#include "isomssb.S"

# Wrap index of slot computation
#
# Sets ZF if result is 0
#
# Params:
#   \base: Base address of container
#   \r0:   Index to wrap
#   \r1:   Scratch register
#
# Return:
#   \r0: The value of \r0 & (base-><ht/hm>_capacity - 1)
.macro wrapq    base, r0, r1
    movq    cap(\base), \r1
    subq    $1, \r1
    andq    \r1, \r0
.endm

# Increment equality cardinality. \base is the
# base address of the container
#
# Params:
#   \base: Base address of container
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
    movq    %rcx, %r14                      # Vacant or vacant mask

    movq    %r13, 0x18(%rsp)
    movq    %rsi, %r13                      # Address of <ht/hm>_curr

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

# Main eq loop. Calls eq # on each slot whose
# corresponding bit is set in %r15d.
#
# Prerequisites:
#   %r9d: 0-based index of low bit in %r15d (i.e. tzcntl %r15d, %r9d)
#   %rbx: Slot index
#   %rbp: Container base address
#   %r12: Element size (or log2(element size) if \pwr2 == 1)
#   %r13: Address of <ht/hm>_curr
#   %r15: Hash match mask
#
# Params:
#   \pwr2:  1 if element size is a power of 2, 0 otherwise
#   \map:   1 if probing a map, 0 if probing a table
#   \label: Loop label
#   \dupl:  Label to jump to on identified duplicate
#
# Return:
#   -
.macro eqloop pwr2, map, label, dupl
    leaq    (%rbx, %r9), %rax               # Array index
    wrapq   %rbp, %rax, %rcx                # Wrap index

.if \map
    movq    pairsz(%rbp), %rdx              # Size of key-value pair
.else
    leaq    1(%rax), %rax                   # Compensate for %r13 holding address of &ht_data[-1]
.endif

.if \pwr2
    movq    %r12, %rcx                      # Shift for offset
    shlq    %cl, %rax
.else
    mulq     %r12                           # Multiply for offset
.endif

.if \map
    leaq    (%rax, %rdx), %rax              # Compensate for size of key-value pair
.endif

    movq    %r13, %rsi                      # Address of value pending insertion
    leaq    (%r13, %rax), %rdi              # Address of potential match

    movq    (%rbp), %rax                    # Call eq
    call    *%rax

    ieqcq   %rbp                            # Increment eq call counter

    testl   %eax, %eax                      # Check return value
    jnz     \dupl

    leal    -1(%r15d), %r8d                 # Clear rightmost set bit
    andl    %r8d, %r15d
    tzcntl  %r15d, %r9d                     # Vector index of next matching slot
    jnc     \label                          # Loop if more matches
.endm

# The actual probing logic. The capacity of the container
# is not considered at iteration - there must always be at
# least two vacant slots.
#
# Prerequisites:
#   %rdi:  Base address of container
#   %rsi:  Address of ht_curr/hm_curr
#   %rdx:  Key size
#   %rcx:  Key hash
#
# Params:
#   \pwr2: 1 if element size is a power of 2, 0 otherwise
#   \map:  1 if probing a hash map, 0 if probing a hash table
#
# Return:
#   %rax: Index of slot, or -1 if element
#         is already present
.macro generic_hash_insert_probe pwr2, map
.if \pwr2
    tzcntq  %rdx, %rdx                      # Element size power of 2, use bit index for efficient mult
.endif

    movq    %rcx, %rax                      # Compute slot
    movq    cap(%rdi), %r11                 # Capacity

    subq    $1, %r11                        # Wrap index
    andq    %r11, %rax

    shrq    $57, %rcx                       # Broadcast high 7 bits of hash to ymm15
    orl     $0x80, %ecx                     # Set MSB for comparison with occupied slots
    vmovd   %ecx, %xmm0
    vpbroadcastb    %xmm0, %ymm15

    vpcmpeqb    %ymm14, %ymm14, %ymm14      # All ones
    vpxor   %ymm13, %ymm13, %ymm13          # All zeroes

    movq    mdoff(%rdi), %r10               # Load metadata offset
    leaq    (%rax, %r10), %rcx              # Offset of metadata entry relative base

    vmovdqu (%rdi, %rcx), %ymm0             # Load ymmword's worth of metadata

    vpxor   %ymm0, %ymm14, %ymm1            # MSB 1 for vacant slots
    vpmovmskb   %ymm1, %ecx                 # Extract bitmask

    vpcmpeqb    %ymm0, %ymm15, %ymm2        # All ones for occupied with matching hash

    vpcmpeqb    %ymm0, %ymm13, %ymm3        # All ones for probe end
    vpmovmskb   %ymm3, %r9d                 # Extract bitmask

    testl   %r9d, %r9d                      # Check for probe end
    jz      8f

    isomssbl    %r9d, %r8d                  # Isolate rigthmost 1 bit

    vpmovmskb   %ymm2, %r8d                 # Extract bitmask

    subl    $1, %r9d                        # Create mask
    andl    %r9d, %r8d                      # Mask out matches beyond probe end
    jz      7f

    framesetup                              # Must call eq at least once
    tzcntl  %r8d, %r9d                      # Vector offset of first hash match

0: # At least one match, end (and vacant) in vector
    eqloop  \pwr2, \map, 0b, 6f             # Equality loop

    tzcntl  %r14d, %edx                     # No match, compute offset in vector
    leaq    (%rbx, %rdx), %rax              # Return value
    wrapq   %rbp, %rax, %rbx
    framerst
    vzeroupper
    retq

7: # Vacant found
    tzcntl  %ecx, %edx                      # Offset in vector
    leaq    (%rax, %rdx), %rax
    wrapq   %rdi, %rax, %rcx
    vzeroupper
    retq

6: # Duplicate found
.if \map
    tzcntl  %r15d, %ecx                     # Offset in vector
    leaq    (%rbx, %rcx), %rax              # Compute and wrap index
    wrapq   %rbp, %rax, %rdi

    movq    $0x8000000000000000, %rcx       # Set high bit for duplicate
    orq     %rcx, %rax
.else
    movq    $-1, %rax
.endif
    framerst                                # Restore stack frame
    vzeroupper
    retq

8: # No probe end
    vpmovmskb   %ymm2, %r8d                 # Extract bitmask

    leaq    -1(%rax), %r9                   # Compute index of entry to be probed last
    andq    %r11, %r9                       # Wrap

    cmpq    $vecsize, %r9                   # Check if duplicate in guard

    leaq    (%r10, %r9), %r10               # Offset of metadata element to be probed last
    movzbl  (%rdi, %r10), %r9d              # Load last entry
    movb    $0, (%rdi, %r10)                # Ensure there exists at least one probe end

    jnb     8f                              # Skip if no duplicate in guard

    leaq    1(%r10, %r11), %r11             # Offset of guard entry relative base
    movb    $0, (%rdi, %r11)                # Clear entry in guard
8:
    testl   %ecx, %ecx                      # Check for vacant
    jnz     7f

    testl   %r8d, %r8d                      # Check for match
    jnz     9f

0:
    leaq    0x20(%rax), %rax                # Advance
    wrapq   %rdi, %rax, %r8

    movq    mdoff(%rdi), %r11               # Offset of metadata array
    leaq    (%r11, %rax), %r8               # Offset of metadata entry

    vmovdqu (%rdi, %r8), %ymm0              # Load ymmword

    vpxor   %ymm0, %ymm14, %ymm1            # High bit set for vacant
    vpmovmskb   %ymm1, %ecx                 # Extract bitmask

    vpcmpeqb    %ymm0, %ymm15, %ymm2        # All ones for hash match
    vpmovmskb   %ymm2, %r8d                 # Bitmask

    vpcmpeqb    %ymm0, %ymm13, %ymm3        # All ones for probe end
    vpmovmskb   %ymm3, %r11d                # Bitmask

    testl   %r11d, %r11d                    # Check for probe end
    jnz     8b

    testl   %ecx, %ecx                      # Check for vacant
    jnz     7f

    testl   %r8d, %r8d                      # Check for match
    jz      0b

9: # Match, no vacant
    framesetup                              # Must call eq, set up frame
    shlq    $56, %r9                        # Store end offset in low 7 bytes
    orq     %r10, %r9
    movq    %r9, 0x30(%rsp)                 # End byte and offset to stack

    vmovdqu %ymm15, 0x38(%rsp)              # Preserve broadcasted hash mask

8:
    tzcntl  %r15d, %r9d                     # Vector offset of first match
0:
    eqloop  \pwr2, \map, 0b, 1f

    vmovdqu 0x38(%rsp), %ymm15              # Restore broadcasted hash
    vpcmpeqb    %ymm14, %ymm14, %ymm14      # All ones
    vpxor   %ymm13, %ymm13, %ymm13          # All zeroes

5:
    leaq    0x20(%rbx), %rbx                # Advance
    wrapq   %rbp, %rbx, %rcx

    movq    mdoff(%rbp), %r8                # Offset of metadata array relative base
    leaq    (%r8, %rbx), %rax               # Offset of slot relative base

    vmovdqu (%rbp, %rax), %ymm0             # Load metadata

    vpxor   %ymm0, %ymm14, %ymm1            # High bit set for vacant
    vpmovmskb   %ymm1, %r14d                # Extract bitmask

    vpcmpeqb    %ymm0, %ymm15, %ymm2        # All ones for hash match
    vpmovmskb   %ymm2, %r15d                # Bitmask

    vpcmpeqb    %ymm0, %ymm13, %ymm3        # All ones for probe end
    vpmovmskb   %ymm3, %r8d                 # Bitmask

    testl   %r8d, %r8d                      # Check for probe end
    jz      3f

    isomssbl    %r8d, %ecx                  # Isolate rigthmost 1 bit
    subl    $1, %r8d                        # Create mask
    andl    %r8d, %r15d                     # Mask out matches beyond probe end
    jz      4f

    tzcntl  %r15d, %r9d                     # Vector index of first match
2:
    eqloop  \pwr2, \map, 2b, 1f

4: # End in vector, no match
    movq    0x30(%rsp), %rax                # End byte and offset

    movq    %rax, %rdx                      # Extract end byte
    shrq    $56, %rdx

    movq    $0xffffffffffffff, %rcx         # Mask out byte from index
    andq    %rcx, %rax
    leaq    (%rbp, %rax), %rdi              # Address of metadata byte

    movb    %dl, (%rdi)                     # Restore metadata byte

    movq    mdoff(%rbp), %rsi               # Metadata offset
    leaq    vecsize(%rbp, %rsi), %rax       # Offset of first duplicated byte

    cmpq    %rax, %rdi                      # Check if byte was duplicated
    jnb     4f

    movq    cap(%rbp), %rsi                 # Capacity
    movb    %dl, (%rdi, %rsi)               # Restore byte in guard
4:
    tzcntl  %r14d, %r9d                     # Index in vector
    leaq    (%rbx, %r9), %rax
    wrapq   %rbp, %rax, %rcx

    framerst
    vzeroupper
    retq

2: # No vacant
    tzcntl  %r15d, %r9d                     # Index in vector
    jc      5b
    jmp     0b

3: # No probe end
    testl   %r14d, %r14d                    # Check for vacant
    jz      2b
5:
    tzcntl  %r14d, %ecx                     # Vector offset of vacant
    leaq    (%rbx, %rcx), %r14              # Index
    wrapq   %rbp, %r14, %rcx
    jmp     5f

6: # Vacant found, no probe end, no match
    leaq    0x20(%rax), %rax                # Advance and wrap
    wrapq   %rdi, %rax, %r8

    movq    mdoff(%rdi), %r8                # Offset of metadata relative base
    leaq    (%r8, %rax), %r11               # Offset of entry relative base

    vmovdqu (%rdi, %r11), %ymm0             # Load metadata

    vpcmpeqb    %ymm0, %ymm15, %ymm1        # All ones for matching hash
    vpmovmskb   %ymm1, %r8                  # Extract bitmask

    vpcmpeqb    %ymm0, %ymm13, %ymm2        # All ones for probe end
    vpmovmskb   %ymm2, %r11                 # Extract bitmask

    testl   %r11d, %r11d                    # Check for probe end
    jz      8f

    testl   %r8d, %r8d                      # Check for match
    jnz     8f

    movq    %rcx, %rax                      # Return value
    movb    %r9b, (%rdi, %r10)              # Restore metadata entry

    movq    mdoff(%rdi), %rsi               # Offset of metadata
    leaq    vecsize(%rsi), %rsi             # Offset of first byte not mirrored

    movq    cap(%rdi), %rdx
    leaq    (%r10, %rdx), %rdx

    cmpq    %rsi, %r10
    cmovbq  %rdx, %r10

    movb    %r9b, (%rdi, %r10)
    vzeroupper
    retq

7: # Vacant found, no probe end
    tzcntl  %ecx, %r11d                     # Vector offset
    leaq    (%rax, %r11), %rcx              # Index of vacant
    wrapq   %rdi, %rcx, %r11

    testl   %r8d, %r8d                      # Check for match
    jz  6b

8:
    framesetup

    shlq    $56, %r9                        # Store end offset in low 7 bytes
    orq     %r10, %r9
    movq    %r9, 0x30(%rsp)                 # End byte and offset to stack

    vmovdqu %ymm15, 0x38(%rsp)              # Preserve broadcasted hash mask

5:
    tzcntl  %r15d, %r9d                     # Vector offset of first match
    jc      6f
0:
    eqloop  \pwr2, \map, 0b, 1f

    vmovdqu 0x38(%rsp), %ymm15              # Restore broadcasted hash
    vpxor   %ymm13, %ymm13, %ymm13          # All zeroes
6:
    leaq    0x20(%rbx), %rbx                # Advance and wrap
    wrapq   %rbp, %rbx, %rcx

    movq    mdoff(%rbp), %rdi               # Offset of metadata array relative base
    leaq    (%rdi, %rbx), %rsi              # Offset of metadata entry relative base

    vmovdqu (%rbp, %rsi), %ymm0             # Load metadata

    vpcmpeqb    %ymm0, %ymm15, %ymm1        # All ones for occupied with matching hash
    vpmovmskb   %ymm1, %r15d                # Extract bitmask

    vpcmpeqb    %ymm0, %ymm13, %ymm2        # All ones for probe end
    vpmovmskb   %ymm2, %r9d                 # Extract bitmask

    testl   %r9d, %r9d                      # Check for probe end
    jz      5b

    tzcntl  %r15d, %r9d                     # Offset of first match in vector
    jc      6f

0: # Last eq iteration
    eqloop  \pwr2, \map, 0b, 1f

6: # No match, index in %r14
    movq    0x30(%rsp), %rax                # End byte and offset

    movq    %rax, %rdx                      # Extract end byte
    shrq    $56, %rdx

    movq    $0xffffffffffffff, %rcx         # Mask out byte from index
    andq    %rcx, %rax
    leaq    (%rbp, %rax), %rdi              # Address of metadata byte

    movb    %dl, (%rdi)                     # Restore metadata byte

    movq    mdoff(%rbp), %rsi               # Metadata offset
    leaq    vecsize(%rbp, %rsi), %rax       # Offset of first duplicated byte

    cmpq    %rax, %rdi                      # Check if byte was duplicated
    jnb     4f

    movq    cap(%rbp), %rsi                 # Capacity
    movb    %dl, (%rdi, %rsi)               # Restore byte in guard
4:
    movq    %r14, %rax                      # Return value
    framerst                                # Restore stack
    vzeroupper
    ret

1: # Duplicate found
    movq    0x30(%rsp), %rax                # End byte and offset

    movq    %rax, %rdx                      # Extract end byte
    shrq    $56, %rdx

    movq    $0xffffffffffffff, %rcx         # Mask out byte from index
    andq    %rcx, %rax
    leaq    (%rbp, %rax), %rdi              # Address of metadata byte

    movb    %dl, (%rdi)                     # Restore metadata byte

    movq    mdoff(%rbp), %rsi               # Metadata offset
    leaq    vecsize(%rbp, %rsi), %rax       # Offset of first duplicated byte

    cmpq    %rax, %rdi                      # Check if byte was duplicated
    jnb     4f

    movq    cap(%rbp), %rsi                 # Capacity
    movb    %dl, (%rdi, %rsi)               # Restore byte in guard
4:
.if \map
    tzcntl  %r15d, %ecx                     # Offset in vector
    leaq    (%rbx, %rcx), %rax              # Return value
    wrapq   %rbp, %rax, %rdi

    movq    $0x8000000000000000, %rcx       # Set high bit for duplicate
    orq     %rcx, %rax
.else
    movq    $-1, %rax
.endif
    framerst                                # Restore stack
    vzeroupper
    ret
.endm
