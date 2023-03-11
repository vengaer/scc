#ifndef AVX2_TRAMPOLINE_H
#define AVX2_TRAMPOLINE_H

# Determine whether avx2 is supported. If it is,
# tail-call \supp. If it is not, tail-call \nosupp
#
# Params:
#   \supp:   Function to call if avx2 is supported
#   \unsupp: Function to call if avx2 is not supported
#
# Return:
#   Whatever \supp or \unsupp return
.macro avx2_trampoline supp, nosupp
    movq    scc_avx2_support@GOTPCREL(%rip), %rax
    movl    (%rax), %eax
    subl    $0x01, %eax
    jz      \supp
    jc      \nosupp
    call    scc_impl_have_avx2
    jz      \supp
    jmp     \nosupp
.endm

#endif /* AVX2_TRAMPOLINE_H */
