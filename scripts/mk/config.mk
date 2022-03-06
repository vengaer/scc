ifndef __Config_mk
__Config_mk := _

include $(mkscripts)/expr.mk

# Instrumentation
SCC_EXPAND_ASM   := $(call is_set,$(CONFIG_EXPAND_ASM))
CPPFLAGS         += $(if $(call is_set,$(CONFIG_PERFEVENTS)),-DSCC_PERFEVTS)

# Fuzzing
SCC_FUZZ_TARGET  := $(CONFIG_FUZZ_TARGET)
SCC_FUZZTIME     := $(CONFIG_FUZZ_TIME)
SCC_FUZZLEN      := $(CONFIG_FUZZ_LENGTH)
SCC_FUZZTIMEO    := $(CONFIG_FUZZ_TIMEOUT)
SCC_FUZZ_CORPORA := $(CONFIG_FUZZ_CORPORA)

# Benchmarking
SCC_BM_TARGET    := $(CONFIG_BENCHMARK_TARGET)
SCC_BM_TYPE      := $(CONFIG_BENCHMARK_TYPE)

endif