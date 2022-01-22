ifndef __Stack_mk
__Stack_mk := _

$(if $(mkscripts),,$(error mkscritps is empty))
include $(mkscripts)/expr.mk

__stack_top_sym        := >>:
__stack_join_sym       := <<>>
__stack_empty_ent      := <<-
__space                := $(subst ,, )

__stack-assert-nonempty = $(call assert,$(call not,$(call stack-empty,$(1))),Stack is empty)

stack-init              = $(eval $(1) := $(__stack_top_sym)$(2))
stack-empty             = $(call not,$(subst $(__stack_top_sym),,$($(strip $(1)))))
stack-push              = $(eval $(1) := $(__stack_top_sym)$(subst $(__space),$(__stack_join_sym),$(strip $(if $(2),$(2),$(__stack_empty_ent)))) $(subst $(__stack_top_sym),,$($(strip $(1)))))
stack-pop               = $(call __stack-assert-nonempty,$(1)) \
                          $(eval $(1) := $(__stack_top_sym)$(filter-out $(firstword $($(strip $(1)))),$($(strip $(1)))))
stack-top               = $(strip $(call __stack-assert-nonempty,$(1)) \
                              $(subst $(__stack_empty_ent),,$(subst $(__stack_top_sym),,$(subst $(__stack_join_sym), ,$(firstword $($(strip $(1))))))))

__dstack_add_prefix   := ||+||
__dstack_sub_prefix   := ||-||

dstack-init             = $(call stack-init,__dstack_$(1))$(eval __dstack_$(1)_top := $(2))
dstack-top              = $(__dstack_$(1)_top)
dstack-dump             = $(info $(__dstack_$(1)))

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

endif # __Stack_mk
