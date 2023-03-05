    .section .data

scc_avx2_hardware_support:                                      # Global indicating avx2 support
    .long 2                                                     # 0 - avx2 supported
                                                                # 1 - avx2 unsupported
                                                                # 2 - unknown, must check

    .section .text

.global scc_impl_have_avx2
# Check for avx2 support. The value is cached upon query
#
# Params:
#
# Return:
#   %eax: 1 if avx2 is supported, 0 otherwise
scc_impl_have_avx2:
    movl    $0x01, %eax
    subl    scc_avx2_hardware_support(%rip), %eax               # Set CF if check is required
    jc      1f
    retq
1:
    movq    %rbx, %r8                                           # Preserve rbx past cpuid

    movl    $0x01, %eax                                         # Request version information
    cpuid

    movq    %r8, %rbx                                           # Restore rbx

    testl   $0xc000000, %ecx                                    # osxsave and avx feature flags

    movl    $0x01, %ecx                                         # Unsupported, set scc_avx2_hardware_support to 1
    jz      1f

    xorl    %ecx, %ecx
    xgetbv                                                      # Read extended control register 0

    testl   $0x03, %eax                                         # xmm and ymm state support

    movl    $0x01, %ecx                                         # Unsupported, set scc_avx2_hardware_support to 1
    jz      1f

    xorl    %ecx, %ecx                                          # Supported, set scc_avx2_hardware_support to 0
1:
    movl    $0x02, %eax                                         # Had to check, expecting scc_avx2_hardware_support == 2
    lock
    cmpxchg %ecx, scc_avx2_hardware_support(%rip)               # Atomic CAS, fails only if another thread already set the
                                                                # (correct) value
    xorl    $0x01, %ecx                                         # Function expected to return 1 when supported
    movl    %ecx, %eax
    retq
