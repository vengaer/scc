ifndef __node
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	$(MAKE) -C $(CURDIR)/.. --no-print-directory -s
endif

# TODO: Requires automatic backup of flags variables. This should
# really be done using some sort of refcounted stack to avoid
# hogging huge amounts of memory
CFLAGS  += -fPIC
LDFLAGS += -shared -Wl,soname,lib$(scc).$(soext).$(socompat)

__node_obj := $(patsubst $(__node_path)/%.$(cext),$(__node_builddir)/%.$(oext),$(wildcard $(__node_path)/*.$(cext)))

$(solink): $(__node_obj) | $(builddir)
	$(info [LN] $(notdir $@))
	$(LN) $(LNFLAGS) $< $@

$(solib): $(__node_obj) | $(builddir)
	$(info [LD] $(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(alib): $(__node_obj) | $(builddir)
	$(info [AR] $(notdir $@))
	$(AR) $(ARFLAGS) -o $@ $^

$(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	$(info [CC] $(notdir $@))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)
