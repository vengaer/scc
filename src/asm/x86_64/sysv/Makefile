ifdef __node

__node_obj  := $(call wildcard-obj,$(__node_path),$(asext))
asmexpander := $(pyscripts)/asmexpander.py

$(solib) $(alib): $(__node_obj)

__srcdir := $(if $(call is_set,$(CONFIG_EXPAND_ASM)),\
                $(__node_builddir),                  \
              $(__node_path))

$(__node_builddir)/%.$(oext): $(__srcdir)/%.$(asext) | $(__node_builddir)
	$(info [AS]   $(notdir $@))
	$(AS) -o $@ $< $(ASFLAGS)

.SECONDARY: $(call wildcard-artifacts,$(__node_path),$(asext),$(asext))

$(__node_builddir)/%.$(asext): $(__node_path)/%.$(asext) | $(__node_builddir)
	$(info [PY]   $(notdir $@))
	$(asmexpander) -o $@ $<

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
