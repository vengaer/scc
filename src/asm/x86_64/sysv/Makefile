ifdef __node

__node_obj  := $(call wildcard-obj,$(__node_path),$(asext))
asmexpander := $(pyscripts)/asmexpander.py
asmsrcdir   := $(__node_path)

$(solib) $(alib): $(__node_obj)

__srcdir := $(if $(SCC_EXPAND_ASM),\
                $(__node_builddir),\
              $(__node_path))

$(__node_builddir)/%.$(oext): $(__srcdir)/%.$(asext) | $(__node_builddir)
	$(call echo-as,$(notdir $@))
	$(AS) -o $@ $< $(ASFLAGS) $(CPPFLAGS) -I$(asmsrcdir)

.SECONDARY: $(call wildcard-artifacts,$(__node_path),$(asext),$(asext))

$(__node_builddir)/%.$(asext): $(__node_path)/%.$(asext) | $(__node_builddir)
	$(call echo-gen,$(notdir $@))
	$(asmexpander) -o $@ $<

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
