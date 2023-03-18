ifdef __node

.PHONY: benchmark
benchmark: $$(call require,benchmark,SCC_BM_TARGET)
benchmark: $$(call require,benchmark,SCC_BM_TYPE)

define decl-benchmark
$(eval
    $(eval benchmarktarget := $(__node_builddir)/benchmark_$(__node))
    $(eval __node_obj      := $(strip
                                  $(call wildcard-obj,$(__node_path),$(cext))
                                  $(call wildcard-obj,$(__node_path),$(cxxext))))

    $(__node_builddir)/%.$(oext): $(srcdir)/%.$(cext) | $(__node_builddir)
	    $$(call echo-cc,$(__node)/$$(notdir $$@))
	    $(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	    $$(call echo-cc,$(__node)/$$(notdir $$@))
	    $(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__node_builddir)/%.$(oext): $(__node_path)/%.$(cxxext) | $(__node_builddir)
	    $$(call echo-cxx,$(__node)/$$(notdir $$@))
	    $(CXX) -o $$@ $$< $(CXXFLAGS) $(CPPFLAGS)

    $(__node_builddir)/%.$(oext): $(asdir)/%.$(asext) | $(__node_builddir)
	    $$(call echo-as,$(__node)/$$(notdir $$@))
	    $(AS) -o $$@ $$< $(ASFLAGS) $(CPPFLAGS)

    $(benchmarktarget): $(__node_obj) $(alib) $(benchmarkalib) $(benchmarkmain_alib)
	    $$(call echo-ld,$(__node)/$$(notdir $$@))
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

__bm_root := $(__node_path)

CFLAGS    += -O3
CXXFLAGS  += -O3
CPPFLAGS  += -DSCC_BM_TYPE=$(SCC_BM_TYPE) -isystem$(benchmarkdir)/include -I$(__bm_root)
LDFLAGS   += -L$(benchmarkbuilddir)/src
LDLIBS    += -lbenchmark -lbenchmark_main -pthread
LD        := $(CXX)

$(call include-node,$(SCC_BM_TARGET))

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
