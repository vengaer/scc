panicbuilddir := $(builddir)/panic
dirs          += $(panicbuilddir)
panicparser   := $(pyscripts)/testparser.$(pyext)

panicmanifest := $(panicbuilddir)/manifest.$(mkext)
panicvar      := panicgen

panicobj      := $(patsubst $(panictestdir)/%.$(cext),$(panicbuilddir)/%.$(oext),$(wildcard $(panictestdir)/*.$(cext)))

__chk_panic   := $(panicbuilddir)/.chk.stamp

.PHONY: all
all:

$(panicmanifest): $(wildcard $(panictestdir)/*.$(cext)) | $(panicbuilddir)
	$(info [PY] $(notdir $@))
	$(foreach __f,$^,$(PYTHON) $(panicparser) -m$(panicvar) $(__f) -o $(dir $@);)

$(panicbuilddir)/%.$(oext): $(panictestdir)/%.$(cext) | $(panicbuilddir)
	$(info [CC] $(notdir $@))
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

$(panicbuilddir)/%.$(oext): $(panicbuilddir)/%.$(cext)
	$(info [CC] $(notdir $@))
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

$(__chk_panic): $(wildcard $(mkscripts)/*.$(mkext))
$(__chk_panic): $(wildcard $(pyscripts)/*.$(pyext))
$(__chk_panic): $(wildcard $(panictestdir)/*.$(pyext))
$(__chk_panic): $(wildcard $(panictestdir)/*.$(cext))
$(__chk_panic): Makefile
	$(PYTEST) $(PYTESTFLAGS) --builddir=$(panicbuilddir) $(panictestdir)
	$(TOUCH) $@

.PHONY: check
check: $(__chk_panic)
check: obj += $(panicobj)

define panic-linker-rules
$(strip
    $(foreach __o,$(patsubst %.$(cext), %.$(oext),$($(panicvar))),
        $(eval
            $(eval __bin := $(panicbuilddir)/$(basename $(notdir $(__o))))
            $(__bin): $(__o) $(panicobj) $(alib)
	            $$(info [LD] $$(notdir $$@))
	            $$(LD) -o $$@ $$^ $$(LDFLAGS) $$(unit_LDFLAGS) $$(LDLIBS)

            $(__chk_panic): $(__bin))))
endef


$(if $(findstring check,$(MAKECMDGOALS)),$(eval -include $(panicmanifest)))
$(call panic-linker-rules)
