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
.macro isomssbl  r0, r1
    leal    -1(\r0), \r1
    notl    \r1
    andl    \r1, \r0
.endm

