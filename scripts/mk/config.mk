ifndef __Config_mk
__Config_mk := _

include $(mkscripts)/expr.mk
include $(mkscripts)/refl.mk

CONFTOOL          := conftool

__validate_config := $(builddir)/.config.valid.stamp

$(root)/.config:
	$(call echo-gen,$(notdir $@))
	$(CONFTOOL) -c $@ generate defconfig

$(__validate_config): $(root)/.config $(__all_mkfiles) | $(builddir)
	$(CONFTOOL) -c $< validate
	$(TOUCH) $@

__simd_enable := $(call is-set,$(CONFIG_SIMD))

# Instrumentation
SCC_EXPAND_ASM   := $(call is-set,$(CONFIG_EXPAND_ASM))
CPPFLAGS         += $(if $(call is-set,$(CONFIG_PERFEVENTS)),-DSCC_PERFEVTS)
CPPFLAGS         += $(if $(call is-set,$(CONFIG_FUZZ_DEBUG)),-DSCC_FUZZ_DEBUG)
CPPFLAGS         += $(if $(call is-set,$(CONFIG_BENCHMARK_STD)),-DSCC_BENCHMARK_STD)

# Fuzzing
SCC_FUZZ_TARGET  := $(CONFIG_FUZZ_TARGET)
SCC_FUZZTIME     := $(CONFIG_FUZZ_TIME)
SCC_FUZZLEN      := $(CONFIG_FUZZ_LENGTH)
SCC_FUZZTIMEO    := $(CONFIG_FUZZ_TIMEOUT)
SCC_FUZZ_CORPORA := $(CONFIG_FUZZ_CORPORA)

# Benchmarking
SCC_BM_TARGET    := $(CONFIG_BENCHMARK_TARGET)
SCC_BM_TYPE      := $(CONFIG_BENCHMARK_TYPE)

# Build options
VERBOSE          ?= $(call is-set,$(CONFIG_VERBOSE))

endif
