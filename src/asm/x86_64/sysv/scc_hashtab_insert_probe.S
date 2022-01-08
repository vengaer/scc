
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

    movq    cap(%rbx), %r8                  # Table capacity
    subq    $1, %r8                         # Mask for finding slot

    movq    %rcx, %rax                      # Compute slot
    andq    %r8, %rax

    movq    %rcx, %r8                       # 14 high-bit hash
    shrq    $50, %r8

    vmovd   %r8d, %xmm0                     # Broadcast hash to ymm15
    vpbroadcastw    %xmm0, %ymm15

.Lprobe:
    movq    mdoff(%rbx), %r8                # Offset of metadata array
    leaq    (%r8, %rax, 2), %rdi            # Offset of slot in metadata array
    leaq    (%rbx, %rdi), %rcx              # Address of slot

    vpcmpeqb    %ymm14, %ymm14, %ymm14      # Mask for hash comparison
    vpsrlw  $2, %ymm14, %ymm14

    vmovdqu (%rcx), %ymm0                   # Load ymmword at offset

    vpand   %ymm0, %ymm14, %ymm1            # Mask out control bits

    vpcmpeqb    %ymm13, %ymm13, %ymm13      # One's complement mask

    vpsllw  $1, %ymm0, %ymm2                # Compute xnor of control bits
    vpxor   %ymm0, %ymm2, %ymm3
    vpxor   %ymm3, %ymm13, %ymm2            # High bit 1 for vacant slot

    vpcmpeqw    %ymm1, %ymm15, %ymm4        # Compare hashes
    vpand   %ymm4, %ymm0, %ymm3             # High bit one if hashes match

    vpmovmskb   %ymm2, %r8d                 # Extract bitmasks
    andl    $0xaaaaaaaa, %r8d               # Mask out bits corresponding to low bytes

    vpmovmskb   %ymm3, %r9d
    andl    $0xaaaaaaaa, %r9d

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

    shrl    $1, %ecx                        # Divide by to for word offset
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

    shrl    $1, %ecx                        # Divide by 2 for word offset
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
    addq    $0x20, %rax                     # Addvance index
    movq    cap(%rbx), %rdi                 # Capacity
    subq    $1, %rdi                        # Wrap index
    andq    %rdi, %rax
    jmp     .Lprobe

.Lfound:
    movq    0x40(%rsp), %rbx                # Restore rbx
    addq    $framesz, %rsp                  # Restore stack
    shrl    $1, %r10d                       # Divide by 2 for word offset
    leaq    (%rax, %r10), %rax              # Compute index
    vzeroupper
    ret

.Lduplicate:
    movq    0x40(%rsp), %rbx                # Restore rbx
    addq    $framesz, %rsp                  # Restore stack
    movq    $-1, %rax                       # Return -1
    vzeroupper
    ret
