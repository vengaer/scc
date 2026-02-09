ifdef __node

archscript  := $(pyscripts)/arch.$(pyext)

# General platform information
archinfo    := $(__node_builddir)/archinfo.$(mkext)

$(archinfo): $(archscript) | $(__node_builddir)
	$(call echo-gen,$(notdir $@))
	$(PYTHON) $< -o $@ $(CC)

ifneq ($(call not,$(__skip_arch)),)
  -include $(archinfo)
endif

ifneq ($(arch_lower),)
  ifneq ($(wildcard $(__node_path)/$(arch_lower)),)
    $(call include-node,$(arch_lower))
  endif
endif

else

# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
