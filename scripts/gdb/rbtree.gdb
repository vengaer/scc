# Print value in node
# arg0 - value type
# arg1 - address of node
# arg2 - tree
define nodeval
    print *($arg0 *)((unsigned char *)$arg1 + $arg2->rb_baseoff)
end

# Print value in tree root
# arg0 - value type
# arg1 - tree
define rootval
    nodeval $arg0 $arg1->rb_sentinel.un_link.node.left $arg1
end

# Print value in node without reference to tree
# arg0 - value type
# arg1 - node
# arg2 - value offset in node
define val
    print *($arg0 *)((unsigned char *)$arg1 + $arg2)
end

# Like nodeval but assume value type is unsigned
# arg0 - address of node
# arg1 - tree
define unodeval
    nodeval unsigned $arg0 $arg1
end

# Like rootval but assume value type is unsigned
# arg0 - tree
define urootval
    rootval unsigned $arg0
end

# Like val but assume value type is unsigned and
# platform is amd64
# arg0 - node
define uval
    val unsigned $arg0 24
end
