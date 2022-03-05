ifdef __node

.PHONY: benchmark
benchmark: $$(call require,benchmark,CONFIG_BENCHMARK)
benchmark: $$(call require,benchmark,CONFIG_BENCHMARK_TYPE)

define decl-benchmark
$(eval
    $(eval benchmarktarget := $(__node_builddir)/benchmark_$(__node))
    $(eval __node_obj      := $(call wildcard-obj,$(__node_path),$(cext))                            \
                              $(call wildcard-obj,$(__node_path),$(cxxext))                          \
                              $(patsubst $(srcdir)/%.$(cext),$(__node_builddir)/%.$(oext),           \
                                  $(wildcard $(srcdir)/*$(__node)*.$(cext)))                         \
                              $(patsubst $(srcdir)/%.$(cext),$(__node_builddir)/%.$(oext),           \
                                  $(addsuffix *.$(cext),$(addprefix $(srcdir)/*,$($(__node)_deps)))) \
                              $(patsubst $(asdir)/%.$(asext),$(__node_builddir)/%.$(oext),           \
                                  $(wildcard $(asdir)/*$(__node)*.$(asext))))

    $(__node_builddir)/%.$(oext): $(srcdir)/%.$(cext) | $(__node_builddir)
	    $$(info [CC]   $(__node)/$$(notdir $$@))
	    $(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	    $$(info [CC]   $(__node)/$$(notdir $$@))
	    $(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__node_builddir)/%.$(oext): $(__node_path)/%.$(cxxext) | $(__node_builddir)
	    $$(info [CXX]  $(__node)/$$(notdir $$@))
	    $(CXX) -o $$@ $$< $(CXXFLAGS) $(CPPFLAGS)

    $(__node_builddir)/%.$(oext): $(asdir)/%.$(asext) | $(__node_builddir)
	    $$(info [AS]   $(__node)/$$(notdir $$@))
	    $(AS) -o $$@ $$< $(ASFLAGS)

    $(benchmarktarget): $(__node_obj) $(benchmarkalib) $(benchmarkmain_alib)
	    $$(info [LD]   $(__node)/$$(notdir $$@))
	    $(LD) -o $$@ $$^ $(LDFLAGS) $(LDLIBS)

    benchmark: $(benchmarktarget)
	    $$<)
endef

$(call push,CFLAGS)
$(call push,CXXFLAGS)
$(call push,CPPFLAGS)
$(call push,LDFLAGS)
$(call push,LDLIBS)
$(call push,LD)

CFLAGS   += -O3
CXXFLAGS += -O3
CPPFLAGS += -DSCC_BM_TYPE=$(CONFIG_BENCHMARK_TYPE) -isystem$(benchmarkdir)/include -I$(__node_path)
LDFLAGS  += -L$(benchmarkbuilddir)/src
LDLIBS   += -lbenchmark -lbenchmark_main -pthread
LD       := $(CXX)

$(call include-node,$(CONFIG_BENCHMARK))

$(call pop,LD)
$(call pop,LDLIBS)
$(call pop,LDFLAGS)
$(call pop,CPPFLAGS)
$(call pop,CXXFLAGS)
$(call pop,CFLAGS)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
