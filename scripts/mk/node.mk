ifndef __Node_mk
__Node_mk := _

$(if $(mkscripts),,$(error mkscritps is empty))
include $(mkscripts)/expr.mk
include $(mkscripts)/stack.mk

$(call stack-init,__node_stack)
$(call stack-init,__node_builddir_stack)
$(call stack-init,__obj_stack)

$(call assert,$(builddir),builddir is empty)
$(call assert,$(root),root is empty)

__node_builddir := $(builddir)
__node_path     := $(root)

$(call stack-push,__node_stack,$(lastword $(subst /, ,/$(__node_path))))
$(call stack-push,__node_builddir_stack,$(__node_builddir))

__node_debug     = $(if $(DEBUG),$$(info DEBUG: $(1)))

# Traverse path through $(1), optionally supplying the
# stem of the builddir as $(2)
# $(call __enter-node,NODENAME[,BUILDDIR])
define __enter-node
$(eval
    # Preserve current node and update it
    $(call stack-push,__node_stack,$(__node))
    $(eval __node := $(strip $(1)))
    $(call __node_debug,enter $(__node))

    $(call __node_debug,$(if $(2),builddir: $(2),no builddir suplied))

    # Preserve current build dir and update it
    $(call stack-push,__node_builddir_stack,$(__node_builddir))
    $(eval __node_builddir := $(__node_builddir)/$(strip $(if $(2),$(2),$(1))))
    $(call __node_debug,__node_builddir: $(__node_builddir))

    # Use dirs recipe to create builddir
    $(eval dirs += $(__node_builddir))

    # Update path to node
    $(eval __node_path := $(__node_path)/$(__node))
    $(call __node_debug,__node_path: $(__node_path))

    # Back up node-local objects
    $(call stack-push,__obj_stack,$(__node_obj))
    $(eval __node_obj :=))
endef

define __exit-node
$(eval
    $(call __node_debug,exit $(__node))

    $(eval __node_obj := $(call stack-top,__obj_stack))
    $(call stack-pop,__obj_stack)
    $(call __node_debug,restored obj: $(__node_obj))

    # Restore node path
    $(eval __node_path := $(patsubst %/$(__node),%,$(__node_path)))
    $(call __node_debug,__node_path: $(__node_path))

    # Restore node builddir
    $(eval __node_builddir := $(call stack-top,__node_builddir_stack))
    $(call stack-pop,__node_builddir_stack)
    $(call __node_debug,__node_builddir: $(__node_builddir))

    # Restore node name
    $(eval __node := $(call stack-top,__node_stack))
    $(call stack-pop,__node_stack)
    $(call __node_debug,__node: $(__node)))
endef

define include-node
$(call __enter-node,$(1),$(2))
$(eval
    # Include the Makefile
    $(eval include $(__node_path)/Makefile))
$(call __exit-node)
endef

endif # __Node_mk
