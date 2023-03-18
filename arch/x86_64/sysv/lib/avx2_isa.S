    .section .data

.globl scc_simd_support
    .align 4
scc_simd_support:                                               # Used to cache AVX2 support
    .long 2                                                     #  - 0 -> not supported
                                                                #  - 1 -> supported
                                                                #  - 2 -> unknown, call scc_impl_have_avx2 to set
    .section .text

# Check for avx2 support. The value is cached in scc_avx2_support
# upon query. Intended for use with the avx2_trampoline macro.
#
# May be called concurrently
#
# NB. this does not abide by the System V calling convention. The
# assumptions made are that
#
# * No registers but r10 and r11 are scrach
# * Uses the ZF instead of eax for "return value"
# * rip must be aligned to an 8-byte boundary, 16-byte is
#   optional
#
# Params:
#
# Return:
#   ZF: set if supported
.globl scc_impl_have_avx2
scc_impl_have_avx2:
    movq    %rbx, %r10                                          # cpuid input 01H clobbers %eax, %ebx, %ecx and %edx
    movq    %rcx, %r11
    movq    %rdx, -0x08(%rsp)                                   # Out of scratch, use red zone

    movl    $0x01, %eax                                         # Request version information
    cpuid

    testl   $0xc000000, %ecx                                    # osxsave and avx feature flags

    movl    $0x00, %ecx                                         # Prepare for XCR read, preserve ZF
    jz      1f                                                  # Unsupported scc_simd_support -> 0

    xgetbv                                                      # Read XCR[%ecx]

    testl   $0x03, %eax                                         # xmm and ymm state support

    jz      1f                                                  # Unsupported, scc_simd_support -> 0

    movl    $0x01, %ecx                                         # Supported, scc_simd_support -> 1
1:
    movl    $0x02, %eax                                         # Had to check, expecting scc_simd_support == 2
    movq    scc_simd_support@GOTPCREL(%rip), %rbx
    lock
    cmpxchg %ecx, (%rbx)                                        # Atomic CAS, fails only if another thread already
                                                                # set the correct value

    subl    $0x01, %ecx                                         # Set ZF if supported

    movq    -0x08(%rsp), %rdx                                   # Restore registers
    movq    %r11, %rcx
    movq    %r10, %rbx
    retq
