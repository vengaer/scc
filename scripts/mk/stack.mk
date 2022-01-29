ifndef __Stack_mk
__Stack_mk := _

$(if $(mkscripts),,$(error mkscritps is empty))
include $(mkscripts)/expr.mk

__stack_top_sym        := >>:
__stack_join_sym       := <<>>
__stack_empty_ent      := <<-
__space                := $(subst ,, )

__stack-assert-nonempty = $(call assert,$(call not,$(call stack-empty,$(1))),Stack is empty)

# Initialize a stack with name $(1) with value $(2). $(2) may be empty
# $(call stack-init,NAME[,INIT])
stack-init              = $(eval $(1) := $(__stack_top_sym)$(subst $(__space),$(__stack_join_sym),$(strip $(2))))

# Check if stack $(1) is empty
# $(call stack-empty,STACK)
stack-empty             = $(call not,$(subst $(__stack_top_sym),,$($(strip $(1)))))

# Push a value $(2) onto the stack $(1)
# $(call stack-push,STACK,VALUE)
stack-push              = $(eval $(1) := $(__stack_top_sym)$(subst $(__space),$(__stack_join_sym),$(strip $(if $(2),$(2),$(__stack_empty_ent)))) $(subst $(__stack_top_sym),,$($(strip $(1)))))

# Pop the topmost value off the stack $(1)
# $(call stack-pop,STACK)
stack-pop               = $(call __stack-assert-nonempty,$(1)) \
                          $(eval $(1) := $(__stack_top_sym)$(filter-out $(firstword $($(strip $(1)))),$($(strip $(1)))))

# Get the topmost value of stack $(1)
# $(call stack-top,STACK)
stack-top               = $(strip $(call __stack-assert-nonempty,$(1)) \
                              $(subst $(__stack_empty_ent),,$(subst $(__stack_top_sym),,$(subst $(__stack_join_sym), ,$(firstword $($(strip $(1))))))))

__dstack_add_prefix   := ||+||
__dstack_sub_prefix   := ||-||
__dstack_op_div       := ||_||

# Initialize a diff stack $(1)  with value $(2). $(2) may be empty
# $(call dstack-init,NAME[,INIT])
dstack-init             = $(call stack-init,__dstack_$(1))$(eval __dstack_$(1)_top := $(strip $(2)))

# Get the topmost value of diff stack $(1)
# $(call dstack-top,DSTACK)
dstack-top              = $(__dstack_$(1)_top)

# Write the change log of diff stack $(1) to stdout
# $(call dstack-dump,DSTACK)
dstack-dump             = $(info $(__dstack_$(1)))

# Add words in $(2) to the top of the dstack $(1)
# $(call dstack-add,DSTACK,VALUE)
define dstack-add
$(strip
    $(eval __dstack_diff :=)
    $(foreach __w,$(2),
        $(if $(call not,$(findstring $(__w),$(__dstack_$(1)_top))),
            $(eval __dstack_$(1)_top += $(__w))
            $(eval __dstack_diff += $(__w))))
    $(if $(__dstack_diff),
        $(call stack-push,__dstack_$(1),$(__dstack_add_prefix)$(subst $(__space),$(__stack_join_sym),$(strip $(__dstack_diff))))))
endef

# Remove words in $(2) to the top of the dstack $(1)
# $(call dstack-sub,DSTACK,VALUE)
define dstack-sub
$(strip
    $(eval __dstack_diff :=)
    $(foreach __w,$(2),
        $(if $(findstring $(__w),$(__dstack_$(1)_top)),
            $(eval __dstack_$(1)_top := $(filter-out $(__w),$(__dstack_$(1)_top)))
            $(eval __dstack_diff += $(__w))))
    $(if $(__dstack_diff),
        $(call stack-push,__dstack_$(1),$(__dstack_sub_prefix)$(subst $(__space),$(__stack_join_sym),$(strip $(__dstack_diff))))))
endef

# Push a new value $(2) to the top of diff stack $(1)
# $(call dstack-push,DSTACK,VALUE)
define dstack-push
$(strip
    $(eval __dstack_add := $(call diff,$(2),$(call dstack-top,$(1))))
    $(eval __dstack_sub := $(call diff,$(call dstack-top,$(1)),$(2)))
    $(call stack-push,__dstack_$(1),$(strip
        $(if $(__dstack_add),
            $(__dstack_add_prefix)$(subst $(__space),$(__stack_join_sym),$(strip $(__dstack_add))))$(strip
        )$(__dstack_op_div)$(strip
        $(if $(__dstack_sub),
            $(__dstack_sub_prefix)$(subst $(__space),$(__stack_join_sym),$(strip $(__dstack_sub)))))))
    $(if $(2),$(eval __dstack_$(1)_top := $(2))))
endef

# Undo the last dstack-add or dstack-sub on dstack $(1)
# $(call dstack-pop,DSTACK)
define dstack-pop
$(strip
    $(eval __dstack_chg  := $(subst $(__dstack_op_div), ,$(subst $(__space),$(__stack_join_sym),$(call stack-top,__dstack_$(1)))))
    $(call stack-pop,__dstack_$(1))
    $(eval __dstack_sub := $(filter $(__dstack_sub_prefix)%,$(__dstack_chg)))
    $(if $(__dstack_sub),
        $(eval __dstack_$(1)_top += $(subst $(__stack_join_sym), ,$(subst $(__dstack_sub_prefix),,$(__dstack_sub)))))

    $(eval __dstack_add := $(filter $(__dstack_add_prefix)%,$(__dstack_chg)))
    $(if $(__dstack_add),
        $(foreach __w,$(subst $(__stack_join_sym), ,$(subst $(__dstack_add_prefix),,$(__dstack_add))),
            $(eval __dstack_$(1)_top := $(filter-out $(__w),$(__dstack_$(1)_top))))))
endef

push = $(call stack-push,__global_stack,$($(1)))
pop  = $(eval $(1) := $(strip $(call stack-top,__global_stack)$(call stack-pop,__global_stack)))

$(call stack-init,__global_stack)

endif # __Stack_mk
