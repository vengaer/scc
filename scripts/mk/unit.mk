unitydir     := $(root)/unity
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

unity.stamp   = $(unitydir)/.unity.stamp

$(unity.stamp): | $(unitydir) $(unitbuilddir)
	git submodule update --init
	$(TOUCH) $(TOUCHFLAGS) $@

$(unityalib): $(unity.stamp)
	$(CMAKE) -B $(dir $@) $(dir $@)
	$(MAKE) -C $(dir $@)
	$(TOUCH) $@

$(unitygen): $(unity.stamp)

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
check: CPPFLAGS += -I$(unityinc) -I$(inspectdir)
check: obj      += $(unitobj)

.PHONY: lint
lint:  CPPFLAGS += -I$(unityinc)

define check-linker-rules
$(strip
    $(foreach __o,$(unitobj),
        $(eval
            $(eval __bin := $(unitbuilddir)/$(basename $(notdir $(__o))))
            $(__bin): $(__o) $(patsubst %.$(oext),%$(runnersuffix).$(oext),$(__o)) $(unityalib) $(alib) $(libinspect)
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
distclean: $(if $(wildcard $(unity.stamp)),__clean_unity)

$(call check-linker-rules)
