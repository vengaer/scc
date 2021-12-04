# arg0 - value type
# arg1 - address of node
# arg2 - tree
define nodeval
    print *($arg0 *)((unsigned char *)$arg1 + $arg2->rb_baseoff)
end
