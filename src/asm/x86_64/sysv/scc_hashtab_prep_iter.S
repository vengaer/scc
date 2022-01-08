
.macro  transform r0, r1, r2
    vpsllw  $1, \r0, \r1                    # Overlay tomb and vacant bits
    vpxor   \r0, \r1, \r2                   # High bit 1 if occupied or tombstone
    vpand   \r0, \r2, \r1                   # Mask out tombstones

    vpsrlw  $15, \r1, \r0                   # High bit to low for each word
.endm

# Prepare hash table for fast iteration,
# destroying the metadata array in the
# process
# Params:
#   %rdi: Address of metadata array
#   %rsi: Size of array
# Returns:
#   -
.globl scc_hashtab_impl_prep_iter
scc_hashtab_impl_prep_iter:
    shlq    $1, %rsi                        # Multiply by 2 for word size
    leaq    (%rdi, %rsi), %rdi              # Address of end

    movq    %rsi, %rdx                      # Compute number of vectors
    shrq    $5, %rdx
    cmpq    $1, %rdx                        # Check if unrolling is possible
    jna     .Lresidual

    negq    %rsi                            # Negate offset
.Ltransform_unrolled:
    vmovdqu (%rdi, %rsi), %ymm0             # Load first ymmword
    transform   %ymm0, %ymm1, %ymm2
    vmovdqu %ymm0, (%rdi, %rsi)             # Write back

    vmovdqu 0x20(%rdi, %rsi), %ymm3         # Second ymmword
    transform   %ymm3, %ymm4, %ymm5
    vmovdqu %ymm3, 0x20(%rdi, %rsi)         # Write back

    addq    $0x40, %rsi                     # Advance
    jnz     .Ltransform_unrolled

    btl     $0, %edx                        # Check for residual
    jz      .Lepi

.Lresidual:
    vmovdqu -0x20(%rdi), %ymm0              # Final ymmword
    transform   %ymm0, %ymm1, %ymm2
    vmovdqu %ymm0, -0x20(%rdi)              # Write back

.Lepi:
    vzeroupper
    ret
