ifdef __node

unitydir             := $(__node_path)/unity
unityinc             := $(unitydir)/src
unityalib            := $(unitydir)/libunity.a
unitygenerator       := $(unitydir)/auto/generate_test_runner.rb

benchmarkdir         := $(__node_path)/benchmark
benchmarkbuilddir    := $(benchmarkdir)/build
dirs                 += $(benchmarkbuilddir)

benchmarkalib        := $(benchmarkbuilddir)/src/libbenchmark.a
benchmarkmain_alib   := $(patsubst %.a,%_main.a,$(benchmarkalib))
benchmark_buildflags := -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on

$(submodules):
	git submodule update --init
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

$(benchmarkalib) $(benchmarkmain_alib)&: $(submodules) | $(benchmarkbuilddir)
	$(CMAKE) -E chdir $(benchmarkbuilddir) $(CMAKE) $(benchmark_buildflags) $(benchmarkdir)
	$(CMAKE) --build $(benchmarkbuilddir) --parallel $(cpu_cores) --config Release

.PHONY: __clean_benchmark
__clean_benchmark:
	$(CMAKE) --build $(benchmarkbuilddir) --target clean

.PHONY: distclean
distclean: $(if $(wildcard $(submodules)),__clean_unity)
distclean: $(if $(wildcard $(submodules)),__clean_benchmark)
	rm -f $(submodules)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
