ifdef __node

unitydir             := $(__node_path)/unity
unityinc             := $(unitydir)/src
unityalib            := $(unitydir)/libunity.a
unitygenerator       := $(unitydir)/auto/generate_test_runner.rb

__cmake_cache        := CMakeCache.txt

benchmarkdir         := $(__node_path)/benchmark
benchmarkbuilddir    := $(benchmarkdir)/build
dirs                 += $(benchmarkbuilddir)

benchmarkalib        := $(benchmarkbuilddir)/src/libbenchmark.a
benchmarkmain_alib   := $(patsubst %.a,%_main.a,$(benchmarkalib))
benchmark_buildflags := -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on

mulldir              := $(__node_path)/mull
mullbuilddir         := $(mulldir)/build
dirs                 += $(mullbuilddir)
mull_runner          := $(mullbuilddir)/tools/mull-runner/mull-runner
mull_ir_frontend     := $(mullbuilddir)/tools/mull-ir-frontend/mull-ir-frontend
mull_build_stamp     := $(mullbuilddir)/.build.stamp

$(submodules):
	git submodule update --init --recursive
	$(TOUCH) $@

$(unityalib): $(call reeval,cpu_cores,$(unityalib))
$(unityalib): $(submodules)
	$(CMAKE) -B $(dir $@) $(dir $@)
	$(MAKE) -C $(dir $@) -j$(cpu_cores)
	$(TOUCH) $@

$(unitygenerator): $(submodules)

.PHONY: __clean_unity
__clean_unity:
	$(MAKE) -C $(unitydir) clean --no-print-directory

$(benchmarkalib): $(call reeval,cpu_cores,$(benchmarkalib))

$(benchmarkmain_alib): $(call reeval,cpu_cores,$(benchmarkmain_alib))

$(benchmarkbuilddir)/$(__cmake_cache): $(submodules) | $(benchmarkbuilddir)
	$(CMAKE) -E $(dir $@) $(CMAKE) $(bnechmark_buildflags) $(dir $(dir $@))

$(benchmarkalib) $(benchmarkmain_alib)&: $(benchmarkbuilddir)/$(__cmake_cache)
	$(CMAKE) --build $(benchmarkbuilddir) --parallel $(cpu_cores) --config Release

.PHONY: __clean_benchmark
__clean_benchmark:
	$(if $(wildcard $(benchmarkbuilddir)),$(CMAKE) --build $(benchmarkbuilddir) --target clean)
	$(RM) $(RMFLAGS) $(benchmarkbuilddir)

$(mullbuilddir)/$(__cmake_cache): $(submodules) | $(mullbuilddir)
	$(CMAKE) -E chdir $(mullbuilddir) $(CMAKE) $(mulldir)

$(mull_build_stamp): $(call reeval,cpu_cores,$(mull_build_stamp))
$(mull_build_stamp): $(mullbuilddir)/$(__cmake_cache)
	$(CMAKE) --build $(mullbuilddir) --parallel $(cpu_cores) --config Release
	$(TOUCH) $@

$(mull_runner) $(mull_ir_frontend): $(mull_build_stamp)
	$(LN) $(LNFLAGS) $@-* $@

.PHONY: __clean_mull
__clean_mull:
	$(if $(wildcard $(mullbuilddir)),$(CMAKE) --build $(mullbuilddir) --target clean)
	$(RM) $(RMFLAGS) $(mullbuilddir)

.PHONY: distclean
distclean: $(if $(wildcard $(submodules)),__clean_unity)
distclean: $(if $(wildcard $(submodules)),__clean_benchmark)
distclean: $(if $(wildcard $(submodules)),__clean_mull)
	$(RM) $(RMFLAGS) $(submodules)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
