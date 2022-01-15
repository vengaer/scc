__stack_top_sym := >>:

stack-init       = $(eval $(1) := $(__stack_top_sym))
stack-empty      = $(call not,$(subst $(__stack_top_sym),,$($(1))))
stack-push       = $(eval $(1) := $(__stack_top_sym)$(2) $(subst $(__stack_top_sym),,$($(1))))
stack-pop        = $(call assert,$(call not,$(call stack-empty,$(1)))) \
                   $(eval $(1) := $(__stack_top_sym)$(patsubst $(__stack_top_sym)%,,$($(1))))
