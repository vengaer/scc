
# Probe for empty slot to insert in
# Params:
#   %rdi: Base address of hash table
#   %rsi: Address of ht_tmp
#   %rdx: Element size
#   %rcx: Hash of ht_tmp
# Return:
#   %rax: Offset of slot, or -1 if element
#         already exists
.globl scc_hashtab_impl_insert_probe
scc_hashtab_impl_insert_probe:
.equ    mdoff, 0x10                         # Offset of ht_mdoff in struct
.equ    cap, 0x20                           # Offset of ht_capacity in struct
.equ    framesz, 0x48                       # Size of stack frame

    subq    $framesz, %rsp                  # Space for locals, align stack

    movq    %rbx, 0x40(%rsp)                # Use %rbx for persistently storing table address
    movq    %rdi, %rbx

    movq    cap(%rbx), %rdi                 # Table capacity
    subq    $1, %rdi                        # Mask for finding slot

    movq    %rcx, %rax                      # Compute slot
    andq    %rdi, %rax

    shrq    $50, %rcx                       # 14 high-bit hash
    vmovd   %ecx, %xmm0                     # Broadcast hash to ymm15
    vpbroadcastw    %xmm0, %ymm15

.Lprobe:
    movq    mdoff(%rbx), %r8                # Offset of metadata array
    leaq    (%r8, %rax, 2), %rcx            # Offset of slot in metadata array

    vpcmpeqb    %ymm14, %ymm14, %ymm14      # Mask for hash comparison
    vpsrlw  $2, %ymm14, %ymm14

    vmovdqu (%rbx, %rcx), %ymm0             # Load first ymmword at offset

    vpand   %ymm0, %ymm14, %ymm1            # Mask out control bits

    vpcmpeqb    %ymm13, %ymm13, %ymm13      # One's complement mask

    vpsllw  $1, %ymm0, %ymm2                # Overlay vacant and tombstone bits
    vpxor   %ymm0, %ymm2, %ymm3             # First part of xnor of control bits
    vpor    %ymm0, %ymm2, %ymm5             # High bit set for occupied, tombstone or both
    vpxor   %ymm2, %ymm13, %ymm4            # One's complement
    vpand   %ymm4, %ymm5, %ymm0             # High bit set if occupied
    vpxor   %ymm3, %ymm13, %ymm2            # Complete xnor, high bit 1 for vacant slot

    vpcmpeqw    %ymm1, %ymm15, %ymm4        # Compare hashes
    vpand   %ymm0, %ymm4, %ymm3             # High bit set if occupied and hash matches

    leaq    0x10(%rax), %r9                 # Advance for next ymmword
    andq    %rdi, %r9                       # Wrap
    leaq    (%r8, %r9, 2), %rcx             # Offset of next ymmword

    vmovdqu (%rbx, %rcx), %ymm5             # Second ymmword

    vpand   %ymm5, %ymm14, %ymm6            # Mask out control bits

    vpsllw  $1, %ymm5, %ymm7                # Overlay vacant and tombstone bits
    vpxor   %ymm5, %ymm7, %ymm8             # First part of xnor of control bits
    vpor    %ymm5, %ymm7, %ymm10            # High bit set for occupied, tombstone or both
    vpxor   %ymm7, %ymm13, %ymm9            # One's complement
    vpand   %ymm9, %ymm10, %ymm5            # High bit set for occupied
    vpxor   %ymm8, %ymm13, %ymm7            # Finish xnor, high bit 1 for vacant slot

    vpcmpeqw    %ymm6, %ymm15, %ymm9        # Compare hashes
    vpand   %ymm5, %ymm9, %ymm8             # High bit set if occupied and hash matches

    vpsllw  $15, %ymm13, %ymm12             # High-bit mask

    vpand   %ymm2, %ymm12, %ymm0            # Mask out low 15 bits of each word
    vpand   %ymm3, %ymm12, %ymm1
    vpand   %ymm7, %ymm12, %ymm5
    vpand   %ymm8, %ymm12, %ymm6

    vpsrlw  $1, %ymm0, %ymm2                # Shift down sign bit
    vpsrlw  $1, %ymm1, %ymm3
    vpsrlw  $1, %ymm5, %ymm7
    vpsrlw  $1, %ymm6, %ymm8

    vpackuswb   %ymm7, %ymm2, %ymm0         # Bit 14 of words to high bit of bytes
    vpackuswb   %ymm8, %ymm3, %ymm1

    vpmovmskb   %ymm0, %r8d                 # Extract bitmasks
    vpmovmskb   %ymm1, %r9d

    bsfl    %r8d, %r10d                     # Offset in ymmword for first vacant slot
    jz      .Lno_vacant

    bsfl    %r9d, %ecx                      # Offset in ymmword of first matching hash
    jz      .Lfound                         # Vacant exists, no match, done

    cmpl    %ecx, %r10d                     # Check if vacant before matching hash
    jb      .Lfound

    movl    $1, %r11d                       # Clear hash match bit
    shll    %cl, %r11d
    notl    %r11d
    andl    %r11d, %r9d

    movl    %r10d,(%rsp)                    # Store offset of first vacant slot
    movq    %rax, 0x08(%rsp)                # Store slot index
    movq    %rdx, 0x10(%rsp)                # Store element size
    movq    %rsi, 0x18(%rsp)                # Store ht_tmp

.Leq_probe:
    movl    %r9d, 0x04(%rsp)                # Store hash match bitmask

    leaq    1(%rax, %rcx), %r11             # Array offset, %rdi holds address of ht_data[-1]

    bsfq    %rdx, %rcx                      # Multiply for offset
    shlq    %cl, %r11

    leaq    (%rsi, %r11), %rdi              # Address of slot

    movq    (%rbx), %rax                    # Call eq
    call    *%rax
    btl     $0, %eax

    jc      .Lduplicate

    movl    (%rsp), %r10d                   # Restore offset of first vacant slot
    movl    0x04(%rsp), %r9d                # Restore hash match bitset

    bsfl    %r9d, %ecx                      # Offset in ymmword of next matching hash
    jz      .Lfound                         # Vacant exists, no match, done

    cmpl    %ecx, %r10d                     # Check if vacant before matching hash
    jb      .Lfound

    movq    0x08(%rsp), %rax                # Restore slot index
    movq    0x10(%rsp), %rdx                # Restore element size
    movq    0x18(%rsp), %rsi                # Restore ht_tmp

    movl    $1, %r11d                       # Clear hash match bit
    shll    %cl, %r11d
    notl    %r11d
    andl    %r11d, %r9d
    jmp     .Leq_probe

.Lno_vacant:
    bsfl    %r9d, %ecx                      # Offset in ymmword of first matching hash
    jz      .Lno_match

    movq    %rax, 0x08(%rsp)                # Slot index
    movq    %rdx, 0x10(%rsp)                # Element size
    movq    %rsi, 0x18(%rsp)                # ht_tmp
    vmovdqa %ymm15, 0x20(%rsp)              # Packed hash

.Leq_probe_no_vacant:
    movl    $1, %r11d                       # Clear hash match bit
    shll    %cl, %r11d
    notl    %r11d
    andl    %r11d, %r9d

    movl    %r9d, 0x04(%rsp)                # Hash match bitmask

    leaq    1(%rax, %rcx), %r11             # Array offset, %rdi holds address of ht_data[-1]

    bsfq    %rdx, %rcx                      # Multiply for offset
    shlq    %cl, %r11

    leaq    (%rsi, %r11), %rdi              # Address of slot

    movq    (%rbx), %rax                    # Call eq
    call    *%rax
    btl     $0, %eax

    jc      .Lduplicate

    movl    0x04(%rsp), %r9d                # Hash match bitmask
    movq    0x08(%rsp), %rax                # Restore slot index
    movq    0x10(%rsp), %rdx                # Element size
    movq    0x18(%rsp), %rsi                # ht_tmp

    bsfl    %r9d, %ecx                      # Check if another match exists
    jnz     .Leq_probe_no_vacant

    vmovdqa 0x20(%rsp), %ymm15              # Restore packed hash

.Lno_match:
    addq    $0x20, %rax                     # Advance index
    movq    cap(%rbx), %rdi                 # Capacity
    subq    $1, %rdi                        # Wrap index
    andq    %rdi, %rax
    jmp     .Lprobe

.Lfound:
    movq    0x40(%rsp), %rbx                # Restore rbx
    addq    $framesz, %rsp                  # Restore stack
    leaq    (%rax, %r10), %rax              # Compute index
    vzeroupper
    ret

.Lduplicate:
    movq    0x40(%rsp), %rbx                # Restore rbx
    addq    $framesz, %rsp                  # Restore stack
    movq    $-1, %rax                       # Return -1
    vzeroupper
    ret
