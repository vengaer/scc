bmdir        := $(submoddir)/benchmark
bmbuilddir   := $(bmdir)/build
dirs         += $(bmbuilddir)

bmalib       := $(bmbuilddir)/src/libbenchmark.a
bmalib_main  := $(patsubst %.$(aext),%_main.$(aext),$(bmalib))

bmflags      := -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on

perfbuilddir := $(builddir)/perf/$(SCC_BENCHMARK)
dirs         += $(perfbuilddir)

perfsrcdir   := $(perfdir)/$(SCC_BENCHMARK)
perftarget   := $(perfbuilddir)/benchmark
perfobj      := $(perfbuilddir)/scc_$(SCC_BENCHMARK).$(oext)                                                         \
                $(addprefix $(perfbuilddir)/scc_,$(addsuffix .$(oext),$($(SCC_BENCHMARK)_deps)))                     \
                $(patsubst $(perfsrcdir)/%.$(cext),$(perfbuilddir)/%.$(oext),$(wildcard $(perfsrcdir)/*.$(cext)))    \
                $(patsubst $(perfsrcdir)/%.$(cxxext),$(perfbuilddir)/%.$(oext),$(wildcard $(perfsrcdir)/*.$(cxxext)))

.PHONY: all
all:

$(bmalib): $(call reeval,cpu_cores,$(bmalib))

$(bmalib_main): $(call reeval,cpu_cores,$(bmalib_main))

$(bmalib) $(bmalib_main)&: $(submodules) | $(bmbuilddir)
	$(CMAKE) -E chdir $(bmbuilddir) $(CMAKE) $(bmflags) $(bmdir)
	$(CMAKE) --build $(bmbuilddir) --parallel $(cpu_cores) --config Release

.PHONY: benchmark
benchmark: $$(call require,benchmark,SCC_BENCHMARK)
benchmark: $$(call require,benchmark,SCC_BENCHMARK_TYPE)
benchmark: LD := $(CXX)
benchmark: CPPFLAGS += -DSCC_BM_TYPE=$(SCC_BENCHMARK_TYPE) -isystem$(bmdir)/include
benchmark: LDLIBS   += -lbenchmark -lbenchmark_main -lpthread
benchmark: LDFLAGS  += -L$(bmbuilddir)/src
benchmark: $(perftarget)
	$^

$(perftarget): $(perfobj) $(bmalib) $(bmalib_main)
	$(info [LD] $(notdir $(perfbuilddir))/$(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(perfbuilddir)/%.$(oext): $(srcdir)/%.$(cext) | $(perfbuilddir)
	$(info [CC] $(notdir $(perfbuilddir))/$(notdir $@))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(perfbuilddir)/%.$(oext): $(assrcdir)/%.$(asext) | $(perfbuilddir)
	$(info [AS] $(notdir $(perfbuilddir))/$(notdir $@))
	$(AS) $(ASFLAGS) -o $@ $<

$(perfbuilddir)/%.$(oext): $(perfsrcdir)/%.$(cext) | $(perfbuilddir)
	$(info [CC] $(notdir $(perfbuilddir))/$(notdir $@))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(perfbuilddir)/%.$(oext): $(perfsrcdir)/%.$(cxxext) $(submodules) | $(perfbuilddir)
	$(info [CXX] $(notdir $(perfbuilddir))/$(notdir $@))
	$(CXX) -o $@ $< $(CXXFLAGS) $(CPPFLAGS)
