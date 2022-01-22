ifndef __Volatile_mk
__Volatile_mk := _

$(if $(mkscripts),,$(error mkscritps is empty))
include $(mkscripts)/expr.mk
include $(mkscripts)/stack.mk
__volatile_vars := CFLAGS CPPFLAGS LDFLAGS LDLIBS

$(foreach __v,$(__volatile_vars),$(call dstack-init,__$(__v)_dstack,$($(__v))))

volatile-push = $(foreach __v,$(__volatile_vars),$(call dstack-push,__$(__v)_dstack,$($(__v))))
volatile-pop  = $(foreach __v,$(__volatile_vars),$(eval $(__v) := $(call dstack-top,__$(__v)_dstack))$(call dstack-pop,__$(__v)_dstack))

endif # __Volatile_mk
