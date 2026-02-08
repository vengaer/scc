ifndef __Config_mk
__Config_mk := _

include $(mkscripts)/expr.mk
include $(mkscripts)/ext.mk
include $(mkscripts)/refl.mk

CONFTOOL             := conftool
CONFIG               := $(root)/.config
confbuilddir         := $(builddir)/config
dirs                 += $(confbuilddir)
md5script            := $(pyscripts)/md5sum.py
__config_md5mk       := $(confbuilddir)/md5.$(mkext)

__config_header_init := $(builddir)/.config.init.stamp

-include $(__config_md5mk)
-include $(CONFIG)

config_stamp         := $(confbuilddir)/_$(config_md5)

__conftool_spec      := $(root)/.conftool.$(jsonext)
__validate_config    := $(builddir)/.config.valid.stamp

# Deliberate omit $(__conftool_spec) dependency here to avoid
# overwriting .config
$(CONFIG):
	$(call echo-gen,$(notdir $@))
	$(CONFTOOL) -c $@ -s $(__conftool_spec) generate defconfig

$(__validate_config): $(CONFIG) $(__all_mkfiles) | $(builddir)
	$(CONFTOOL) -c $< validate
	$(TOUCH) $@

$(__config_md5mk): $(CONFIG) $(md5script) | $(confbuilddir)
	$(PYTHON) $(md5script) -f make $< -o $@

$(config_stamp): | $(confbuilddir)
	$(RM) $(RMFLAGS) $(wildcard $(dir $@)/_*)
	$(TOUCH) $@

__devel          := $(call is-set,$(CONFIG_DEVEL))
__devel_docs     := $(call is-set,$(CONFIG_DEVEL_DOCS))

# Instrumentation
SCC_EXPAND_ASM   := $(call is-set,$(CONFIG_EXPAND_ASM))
CPPFLAGS         += $(if $(call is-set,$(CONFIG_PERFEVENTS)),-DSCC_PERFEVTS)
CPPFLAGS         += $(if $(call is-set,$(CONFIG_FUZZ_DEBUG)),-DSCC_FUZZ_DEBUG)
CPPFLAGS         += $(if $(call is-set,$(CONFIG_BENCHMARK_STD)),-DSCC_BENCHMARK_STD)

# Libs

LDLIBS           += $(if $(call is-set,$(CONFIG_MATHDEP)),-lm)
__have_libm      := $(call is-set,$(CONFIG_MATHDEP))

# Fuzzing
SCC_FUZZ_TARGET  := $(CONFIG_FUZZ_TARGET)
SCC_FUZZTIME     := $(CONFIG_FUZZ_TIME)
SCC_FUZZLEN      := $(CONFIG_FUZZ_LENGTH)
SCC_FUZZTIMEO    := $(CONFIG_FUZZ_TIMEOUT)
SCC_FUZZ_CORPORA := $(CONFIG_FUZZ_CORPORA)
SCC_FUZZ_SWAR    := $(call is-set,$(CONFIG_FUZZ_SWAR))

# Benchmarking
SCC_BM_TARGET    := $(CONFIG_BENCHMARK_TARGET)
SCC_BM_TYPE      := $(CONFIG_BENCHMARK_TYPE)

# Build options
VERBOSE          ?= $(call is-set,$(CONFIG_VERBOSE))

__debug_info     := $(if $(call is-set,$(CONFIG_DEBUG_INFO)),-g)
__werror         := $(if $(call is-set,$(CONFIG_WERROR)),-Werror)

endif
