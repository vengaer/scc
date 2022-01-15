$(if $(mkscripts),,$(error mkscritps is empty))
include $(mkscripts)/expr.mk

__stack_top_sym        := >>:
__stack_join_sym       := <<>>
__space                := $(subst ,, )

__stack-assert-nonempty = $(call assert,$(call not,$(call stack-empty,$(1))),Stack is empty)

stack-init              = $(eval $(1) := $(__stack_top_sym))
stack-empty             = $(call not,$(subst $(__stack_top_sym),,$($(1))))
stack-push              = $(eval $(1) := $(__stack_top_sym)$(subst $(__space),$(__stack_join_sym),$(2)) $(subst $(__stack_top_sym),,$($(1))))
stack-pop               = $(call __stack-assert-nonempty,$(1)) \
                          $(eval $(1) := $(__stack_top_sym)$(filter-out $(firstword $($(1))),$($(1))))
stack-top               = $(strip $(call __stack-assert-nonempty,$(1)) \
                              $(subst $(__stack_top_sym),,$(subst $(__stack_join_sym), ,$(firstword $($(1))))))
