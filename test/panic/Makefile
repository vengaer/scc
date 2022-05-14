ifdef __node

__chk_panic   := $(__node_builddir)/.chk.stamp
panicbuilddir := $(__node_builddir)
panictestdir  := $(__node_path)

panicparser   := $(pyscripts)/testparser.$(pyext)
panicmanifest := $(__node_builddir)/manifest.mk
panicsrcvar   := panicsrc

__sccobj      := $(addsuffix .$(oext),$(addprefix $(sccbuilddir)/scc_,svec vec))

$(if $(call eq,$(MAKECMDGOALS),check),\
    $(eval -include $(panicmanifest)))

panicobj      := $(call wildcard-obj,$(__node_path),$(cext))
panicbins     := $(patsubst $(__node_builddir)/%.$(cext),$(__node_builddir)/%,$($(panicsrcvar)))

$(__chk_panic): $(panicbins) $(__all_mkfiles) $(wildcard $(__node_path)/*.$(pyext))
	$(PYTEST) $(PYTESTFLAGS) --builddir=$(panicbuilddir) $(panictestdir)
	$(TOUCH) $@

$(panicmanifest): $(wildcard $(__node_path)/*.$(cext)) | $(__node_builddir)
	$(call echo-gen,$(notdir $@))
	$(PYTHON) $(panicparser) -m$(panicsrcvar) -o $(dir $@) $^

$(panicbins): $$@.$(oext) $(panicobj) $(__sccobj)
	$(call echo-ld,$(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(__node_builddir)/%.$(oext): $(__node_builddir)/%.$(cext) | $(__node_builddir)
	$(call echo-cc,$(notdir $@))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	$(call echo-cc,$(notdir $@))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

check: $(__chk_panic)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
