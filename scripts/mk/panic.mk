panicbuilddir := $(builddir)/panic
dirs          += $(panicbuilddir)
panicparser   := $(pyscripts)/testparser.py

panicmanifest := $(panicbuilddir)/manifest.mk
panicvar      := panicgen

panicobj      := $(patsubst $(panictestdir)/%.$(cext),$(panicbuilddir)/%.$(oext),$(wildcard $(panictestdir)/*.$(cext)))

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

.PHONY: __chk_panic
__chk_panic:
	$(PYTEST) $(PYTESTFLAGS) --builddir=$(panicbuilddir) $(panictestdir)

check: __chk_panic

define panic-linker-rules
$(strip
    $(foreach __o,$(patsubst %.$(cext), %.$(oext),$($(panicvar))),
        $(eval
            $(eval __bin := $(panicbuilddir)/$(basename $(notdir $(__o))))
            $(__bin): $(__o) $(panicobj) $(alib)
	            $$(info [LD] $$(notdir $$@))
	            $$(LD) -o $$@ $$^ $$(LDFLAGS) $$(unit_LDFLAGS) $$(LDLIBS)

            __chk_panic: $(__bin))))
endef


$(if $(findstring check,$(MAKECMDGOALS)),$(eval -include $(panicmanifest)))
$(call panic-linker-rules)
