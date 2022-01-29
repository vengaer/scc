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

push = $(call stack-push,__global_stack,$($(1)))
pop  = $(eval $(1) := $(strip $(call stack-top,__global_stack)$(call stack-pop,__global_stack)))

$(call stack-init,__global_stack)

endif # __Stack_mk
