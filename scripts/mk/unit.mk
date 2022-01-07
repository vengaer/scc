unitydir     := $(submoddir)/unity
unitygen     := $(unitydir)/auto/generate_test_runner.rb
unityinc     := $(unitydir)/src
unityalib    := $(unitydir)/libunity.a

runnersuffix := _gen
runnerdir    := $(builddir)/runner
dirs         += $(runnerdir)

unitbuilddir := $(builddir)/unit
dirs         += $(unitbuilddir)
unitobj      := $(patsubst $(unitdir)/%.$(cext),$(unitbuilddir)/%.$(oext),$(wildcard $(unitdir)/*.$(cext)))

unit_CFLAGS  := -fsanitize=address,undefined
unit_LDFLAGS := -fsanitize=address,undefined

.PHONY: all
all:

$(unityalib): $(call reeval,cpu_cores,$(unityalib))
$(unityalib): $(submodules)
	$(CMAKE) -B $(dir $@) $(dir $@)
	$(MAKE) -C $(dir $@) -j$(cpu_cores)
	$(TOUCH) $@

$(unitygen): $(submodules)

$(runnerdir)/%$(runnersuffix).$(cext): $(unitdir)/%.$(cext) $(unitygen) | $(runnerdir)
	$(info [RB] $(notdir $@))
	$(RB) $(unitygen) $< $@

$(unitbuilddir)/%.$(oext): $(runnerdir)/%.$(cext) | $(unitbuilddir)
	$(info [CC] $(notdir $@))
	$(CC) $(CFLAGS) $(unit_CFLAGS) $(CPPFLAGS) -o $@ $<

$(unitbuilddir)/%.$(oext): $(unitdir)/%.$(cext) | $(unitbuilddir)
	$(info [CC] $(notdir $@))
	$(CC) $(CFLAGS) $(unit_CFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: check
check: CPPFLAGS += -I$(unityinc) -I$(testdir)
check: obj      += $(unitobj)

.PHONY: lint
lint:  CPPFLAGS += -I$(unityinc) -I$(testdir)

define check-linker-rules
$(strip
    $(foreach __o,$(unitobj),
        $(eval
            $(eval __bin := $(unitbuilddir)/$(basename $(notdir $(__o))))
            $(__bin): $(__o) $(patsubst %.$(oext),%$(runnersuffix).$(oext),$(__o)) $(unityalib) $(libinspect) $(alib)
	            $$(info [LD] $$(notdir $$@))
	            $$(LD) -o $$@ $$^ $$(LDFLAGS) $$(unit_LDFLAGS) $$(LDLIBS)

            __chk_$(__bin): $(__bin)
	          $$^

            check: __chk_$(__bin))))
endef

.PHONY: __clean_unity
__clean_unity:
	$(MAKE) -sC $(unitydir) clean

.PHONY: distclean
distclean: $(if $(wildcard $(submodules)),__clean_unity)

$(call check-linker-rules)
