    .section .text

#include <asm/unistd.h>

.equ    O_WRONLY,       0x01
.equ    O_CREAT,        0100
.equ    O_TRUNC,        01000
.equ    STDOUT_FILENO,  1
.equ    STDERR_FILENO,  2

.macro  open, path, flags, mode
    movl    $__NR_open, %eax
    movq    \path, %rdi
    movl    \flags, %esi
    movl    \mode, %edx
    syscall
.endm

.macro  write, fd, str, len
    movq    \str, %rsi
    movl    \len, %edx
    movl    \fd, %edi
    movl    $__NR_write, %eax
    syscall
.endm

.macro exit, ec
    movl    \ec, %edi                                           # Exit status
    movl    $__NR_exit, %eax
    syscall
.endm

.globl main
main:
    movq    %rbx, %r8                                           # Preserve rbx past cpuid

    movl    $0x01, %eax                                         # Request version information
    cpuid

    movq    %r8, %rbx                                           # Restore rbx

    leaq    isa_avx2(%rip), %r8                                 # Load isa avx2
    movl    $isa_avx2_len, %r9d

    leaq    isa_unsup(%rip), %r10                               # Load isa unsupported
    movl    $isa_unsup_len, %r11d

    andl    $0xc000000, %ecx                                    # Check osxsave and avx feature flags
    cmpl    $0xc000000, %ecx
    cmovne  %r10, %r8                                           # Conditionally replace with unsupported
    cmovne  %r11d, %r9d

    xorl    %ecx, %ecx
    xgetbv                                                      # Read extended control register 0

    andl    $0x03, %eax                                         # Check xmm and ymm state support
    cmpl    $0x03, %eax
    cmovne  %r10, %r8                                           # Conditionally replace with unsupported
    cmovne  %r11d, %r9d

    cmpl    $0x01, %edi                                         # Check if output file was specified
    jng     .Lwr_stdout

    open    0x08(%rsi), $O_CREAT | O_WRONLY | O_TRUNC, $0644

    cmpl    $0x00, %eax                                         # open failure?
    jl      .Lopen_failure

    write   %eax, %r8, %r9d

    cmpl    $0x00, %eax                                         # write failure?
    jl      .Lwrite_failure

    movl    $__NR_close, %eax                                   # Close syscall
    syscall

    exit    $0x00

.Lwr_stdout:
    write   $STDOUT_FILENO, %r8, %r9d
    exit    $0x00

.Lopen_failure:
    write   $STDERR_FILENO, open_fail(%rip), $open_fail_len
    exit    $0x01
.Lwrite_failure:
    write   $STDERR_FILENO, write_fail(%rip), $write_fail_len
    exit    $0x01

    .section .data
    isa_avx2:           .ascii "simd_isa := AVX2\n"
    isa_avx2_len =      . - isa_avx2
    isa_unsup:          .ascii "simd_isa := UNSUPPORTED\n"
    isa_unsup_len =     . - isa_unsup
    open_fail:          .ascii "Open syscall failed\n"
    open_fail_len =     . - open_fail
    write_fail:         .ascii "Write syscall failed\n"
    write_fail_len =    . - write_fail
