ifdef __node

CFLAGS     += -fPIC
LDFLAGS    += -shared -Wl,-soname,lib$(scc).$(soext).$(socompat)

__node_obj := $(patsubst $(__node_path)/%.$(cext),$(__node_builddir)/%.$(oext),$(wildcard $(__node_path)/*.$(cext)))

$(call include-node,asm)

$(solink): $(solib) | $(builddir)
	$(info [LN] $(notdir $@))
	$(LN) $(LNFLAGS) $< $@

$(solib): $(call reeval,LDFLAGS,$(solib))
$(solib): $(__node_obj) | $(builddir)
	$(info [LD] $(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(alib): $(__node_obj) | $(builddir)
	$(info [AR] $(notdir $@))
	$(AR) $(ARFLAGS) -o $@ $^

$(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	$(info [CC] $(notdir $@))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory -s
endif
