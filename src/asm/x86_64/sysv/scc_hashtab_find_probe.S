
.macro  setup_frame, size
    subq    \size, %rsp                     # Make space
    movq    %rbx, 0x20(%rsp)                # Must always call eq, use persistent registers for local data
    movq    %rbp, 0x28(%rsp)
    movq    %r12, 0x30(%rsp)
    movq    %r13, 0x38(%rsp)
    movq    %r14, 0x40(%rsp)
    movq    %r15, 0x48(%rsp)
.endm

.macro  restore_frame, size
    movq    0x20(%rsp), %rbx
    movq    0x28(%rsp), %rbp
    movq    0x30(%rsp), %r12
    movq    0x38(%rsp), %r13
    movq    0x40(%rsp), %r14
    movq    0x48(%rsp), %r15
    addq    \size, %rsp
.endm

# Probe for element
# Params:
#   %rdi: Base address of hash table
#   %rsi: Address of ht_tmp
#   %rdx: Element size
#   %rcx: Hash of ht_tmp
# Return:
#   %rax: Offset of slot containin elent,
#         or -1 if element not found
.globl scc_hashtab_impl_find_probe
scc_hashtab_impl_find_probe:
.equ    mdoff, 0x10                         # Offset of ht_mdoff in struct
.equ    cap, 0x20                           # Offset of ht_capacity in struct
.equ    framesz, 0x58                       # 80 bytes of local data, +8 for stack alignment

    setup_frame $framesz

    movq    %rdi, %rbx                      # Persistent table address
    movq    %rsi, %rbp                      # Persistent ht_tmp address
    bsfq    %rdx, %r12                      # Index of low bit, persisten

    movq    cap(%rbx), %rdi                 # Table capacity, power of 2
    subq    $1, %rdi                        # Slot mask

    movq    %rcx, %r13                      # Compute slot
    andq    %rdi, %r13

    shrq    $50, %rcx                       # 14 high-bit hash
    vmovd   %ecx, %xmm0                     # Broadcast hash to ymm15
    vpbroadcastw    %xmm0, %ymm15

.Lprobe:
    movq    mdoff(%rbx), %rax               # Offset of metadata array
    leaq    (%rax, %r13, 2), %rcx           # Offset of slot in metadata array

    vpcmpeqb    %ymm14, %ymm14, %ymm14      # Mask for hash comparison
    vpsrlw  $2, %ymm14, %ymm14

    vmovdqu (%rbx, %rcx), %ymm0             # First ymmword

    vpand   %ymm0, %ymm14, %ymm1            # Mask out control bits

    vpcmpeqb    %ymm13, %ymm13, %ymm13      # One's complement mask

    vpsllw  $1, %ymm0, %ymm2                # Overlay vacant and tombstone bits
    vpor    %ymm0, %ymm2, %ymm3
    vpxor   %ymm3, %ymm13, %ymm0            # One's complement, high bit 1 for probe end

    vpxor   %ymm2, %ymm13, %ymm4            # One's complement
    vpand   %ymm3, %ymm4, %ymm2             # High bit set for occupied slots

    vpcmpeqw    %ymm1, %ymm15, %ymm3        # All bits set for matching hash
    vpand   %ymm2, %ymm3, %ymm1             # High bit set for occupied with matching hash

    leaq    0x10(%r13), %rsi                # Advance for next ymmword
    leaq    (%rax, %rsi, 2), %rcx           # Offset of next ymmword

    vmovdqu (%rbx, %rcx), %ymm5             # Second ymmword

    vpand   %ymm5, %ymm14, %ymm6            # Mask out control bits

    vpsllw  $1, %ymm5, %ymm7                # Overlay vacant and tombstone bits
    vpor    %ymm5, %ymm7, %ymm8
    vpxor   %ymm8, %ymm13, %ymm5            # One's complement, high bit 1 for probe end

    vpxor   %ymm7, %ymm13, %ymm9            # One's complement
    vpand   %ymm8, %ymm9, %ymm7             # High bit set for occupied slots

    vpcmpeqw    %ymm6, %ymm15, %ymm8        # All bits set for matching hash
    vpand   %ymm7, %ymm8, %ymm6             # High bit set for occupied with matching hash

    vpsllw  $15, %ymm13, %ymm12             # High-bit mask

    vpand   %ymm0, %ymm12, %ymm2            # Mask out low 15 bits in each word
    vpsrlw  $1, %ymm2, %ymm0                # Shift down sign bit

    vpand   %ymm1, %ymm12, %ymm3
    vpsrlw  $1, %ymm3, %ymm1

    vpand   %ymm5, %ymm12, %ymm7
    vpsrlw  $1, %ymm7, %ymm5

    vpand   %ymm6, %ymm12, %ymm8
    vpsrlw  $1, %ymm8, %ymm6

    vpackuswb   %ymm5, %ymm0, %ymm2         # Bit 14 of words to high bit of bytes
    vpackuswb   %ymm6, %ymm1, %ymm3

    vpmovmskb   %ymm2, %eax                 # Bit set for stop
    vpmovmskb   %ymm3, %r15d                # Bit set for occupied slot with matching hash

    bsfl    %eax, %r14d                     # Stop slot
    jz      .Lno_stop

    bsfl    %r15d, %ecx                     # First occupied slot with matching hash
    jz      .Lnot_found

    cmpl    %ecx, %r14d                     # Check if stop slot before matching one
    jb      .Lnot_found

    movl    $1, %eax                        # Clear hash match bit
    shll    %cl, %eax
    notl    %eax
    andl    %eax, %r15d

.Leq_probe:
    leaq    1(%r13, %rcx), %rax             # Array offset, %rbp holds address of ht_data[-1]
    movq    %rcx, 0x50(%rsp)                # Preserve offset in ymmword

    movl    %r12d, %ecx                     # Multiply for offset
    shlq    %cl, %rax

    leaq    (%rbp, %rax), %rdi              # Address of slot
    movq    %rbp, %rsi                      # Value searched for

    movq    (%rbx), %rax                    # Call eq
    call    *%rax

    btl     $0, %eax
    jc      .Lfound

    bsfl    %r15d, %ecx                     # Next occupied slot with matching hash
    jz      .Lnot_found

    cmpl    %ecx, %r14d
    jb      .Lnot_found

    movl    $1, %eax                        # Clear hash match bit
    shll    %cl, %eax
    notl    %eax
    andl    %eax, %r15d
    jmp     .Leq_probe

.Lno_stop:
    bsfl    %r15d, %ecx                     # Offset of first matching slot
    jz      .Lkeep_probing

    vmovdqu %ymm15, (%rsp)                  # Preserve packed hash

.Leq_probe_no_stop:
    movl    $1, %eax                        # Clear has match bit
    shll    %cl, %eax
    notl    %eax
    andl    %eax, %r15d

    leaq    1(%r13, %rcx), %rax             # Array offset, %rbp holds address of ht_data[-1]
    movq    %rcx, 0x50(%rsp)                # Preserve offset in ymmword

    movl    %r12d, %ecx                     # Multiply for offset
    shlq    %cl, %rax

    leaq    (%rbp, %rax), %rdi              # Address of slot
    movq    %rbp, %rsi                      # Value searched for

    movq    (%rbx), %rax                    # Call eq
    call    *%rax

    btl     $0, %eax
    jc      .Lfound

    bsfl    %r15d, %ecx                     # Check if another match exists
    jnz     .Leq_probe_no_stop

    vmovdqu (%rsp), %ymm15                  # Restore packed hash

.Lkeep_probing:
    addq    $0x20, %r13                     # Advance offset
    xorl    %edi, %edi
    cmpq    cap(%rbx), %r13
    cmovnbl %edi, %r13d
    jmp     .Lprobe

.Lfound:
    movq    0x50(%rsp), %rcx                # Restore offset in ymmword
    leaq    (%r13, %rcx), %rax              # Compute slot index
    restore_frame $framesz
    vzeroupper
    ret

.Lnot_found:
    restore_frame $framesz
    movq    $-1, %rax                       # Return value
    vzeroupper
    ret
