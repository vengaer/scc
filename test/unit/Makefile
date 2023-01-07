ifdef __node

runnersuffix    := runner

testsccbuilddir := $(__node_builddir)/scc
dirs            += $(testsccbuilddir)

$(call push,CPPFLAGS)
$(call push,LDFLAGS)
$(call push,LDLIBS)

CPPFLAGS        := -I$(unityinc) -I$(testdir) $(filter-out -DNDEBUG,$(CPPFLAGS)) -I$(srcdir) -DSCC_CANARY_ENABLED
LDFLAGS         += -L$(unitydir)
LDLIBS          += -lunity -lm

define __unit-obj
$(strip
    $(patsubst $(srcdir)/%.$(cext),$(testsccbuilddir)/%.$(oext),
        $(wildcard $(patsubst %,$(srcdir)/%*.$(cext),$(__node) $($(__node)_deps))))
    $(patsubst $(inspectdir)/%.$(cext),$(inspectbuilddir)/%.$(oext),
        $(wildcard $(inspectdir)/$(__node)*.$(cext)))
    $(if $(__simd_enable),
        $(patsubst $(asdir)/%.$(asext),$(asbuilddir)/%.$(oext),
            $(wildcard $(patsubst %,$(asdir)/%*.$(asext),$(__node) $($(__node)_deps)))),
      $(patsubst $(simdemudir)/%.$(cext),$(simdemubuilddir)/%.$(oext),
          $(wildcard $(patsubst %,$(simdemudir)/%*.$(cext),$(__node) $($(__node)_deps) swvec))))
    $(if $($(__node)_deps),
        $(patsubst $(inspectdir)/%.$(cext),$(inspectbuilddir)/%.$(oext),
            $(wildcard $(addprefix $(inspectdir)/*,$(addsuffix *.$(cext),$($(__node)_deps)))))))
endef

__mutate-obj = $(addprefix $(__mut_builddir)/,$(notdir $(call __unit-obj)))

define decl-unit
$(eval
    $(call push,CFLAGS)
    $(call push,LDFLAGS)
    $(eval CFLAGS  += -fsanitize=address,undefined)
    $(eval LDFLAGS += -fsanitize=address,undefined)

    $(eval __unitsrc   := $(wildcard $(__node_path)/*.$(cext)))
    $(eval __chk       := $(patsubst $(__node_path)/%.$(cext),$(__node_builddir)/.chk.%.stamp,$(__unitsrc)))
    $(eval __runners   := $(patsubst $(__node_path)/%.$(cext),$(__node_builddir)/%_$(runnersuffix),$(__unitsrc)))
    $(eval __unitobj   := $(call __unit-obj))

    # Prevent make from removing generated runners
    .SECONDARY: $(addsuffix .$(cext),$(__runners))

    # Recipies for running tests
    $(foreach __s,$(__chk),
        $(__s): $(call reeval,CPPFLAGS,$(__s))
        $(__s): $(patsubst $(__node_builddir)/.chk.%.stamp,$(__node_builddir)/%_$(runnersuffix),$(__s))
	        $$<
	        $(TOUCH) $$@)

    # Runner source generation
    $(__node_builddir)/%_$(runnersuffix).$(cext): $(__node_path)/%.$(cext) $(submodules) | $(__node_builddir)
	    $$(call echo-gen,$$(notdir $$@))
	    $(RB) $(unitygenerator) $$< $$@

    $(foreach __r,$(__runners),
        $(__r): $(__r).$(oext) $(patsubst %_$(runnersuffix),%.$(oext),$(__r)) $(__unitobj) $(unityalib)
	        $$(call echo-ld,$$(notdir $$@))
	        $(LD) -o $$@ $$^ $(LDFLAGS))

    $(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	    $$(call echo-cc,$$(notdir $$@))
	    $(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__node_builddir)/%.$(oext): $(srcdir)/%.$(cext) | $(__node_builddir)
	    $$(call echo-cc,$$(notdir $$@))
	    $(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    check: $(__chk)

    $(call pop,LDFLAGS)
    $(call pop,CFLAGS))
endef

define decl-mutate
$(eval
    $(call push,CFLAGS)
    $(call push,CPPFLAGS)
    $(call push,LDFLAGS)
    $(eval CFLAGS += $(__mull_cflags))
    $(eval CPPFLAGS += -DSCC_MUTATION_TEST)
    $(eval LDFLAGS += $(__mull_ldflags))

    $(eval __mut_builddir := $(subst /unit/,/mutate/,$(__node_builddir)))
    $(eval dirs += $(__mut_builddir))

    $(eval __mutsrc     := $(wildcard $(__node_path)/*.$(cext)))
    $(eval __mutstu     := $(__mut_builddir)/$(__node)_all_tests.$(cext))
    $(eval __mut        := $(__mut_builddir)/.mut.$(__node).stamp)
    $(eval __mutrunner  := $(__mut_builddir)/$(__node)_mut$(runnersuffix))

    $(eval __mutstuobj  := $(patsubst %.$(cext),%.$(oext),$(__mutstu)))
    $(eval __mutobj     := $(call __mutate-obj))
    $(eval __mutgendobj := $(__mutrunner).$(oext) $(__mutstu:.$(cext)=.$(oext)))

    $(eval __all_obj    += $(__mutgendobj))

    $(foreach __o,$(__mutobj),
        $(if $(findstring mutate,$(__o)),$(eval __all_obj += $(__o))))

    $(eval __set_mullcfg := MULL_CONFIG=$(__node_path)/mull.yml)

    .SECONDARY: $(__mutstu) $(__mutrunner).$(cext)

    # Running mutation tests
    $(__mut): $(call reeval,CFLAGS,$(__mut))
    $(__mut): $(call reeval,LDFLAGS,$(__mut))
    $(__mut): $(call reeval,CPPFLAGS,$(__mut))
    $(__mut): $(patsubst $(__mut_builddir)/.mut.%.stamp,$(__mut_builddir)/%_mut$(runnersuffix),$(__mut)) $(mull_runner)
	    $(__set_mullcfg) $(mull_runner) $$< $(MULLFLAGS)
	    $(TOUCH) $$@

    # Generate single TU containing all tests
    $(__mutstu): $(pyscripts)/cat.$(pyext) $(__mutsrc) | $(__mut_builddir)
	    $$(call echo-gen,$$(notdir $$@))
	    $(PYTHON) $$< $$(filter-out $$<,$$^) -o $$@

    # Generate runner source
    $(__mutrunner).$(cext): $(__mutstu) $(submodules) | $(__mut_builddir)
	    $$(call echo-gen,$$(notdir $$@))
	    $(RB) $(unitygenerator) $$< $$@

    $(__mutrunner): $(__mutgendobj) $(__mutobj) $(unityalib)
	    $$(call echo-ld,$$(notdir $$@))
	    $(LD) -o $$@ $$^ $(LDLIBS) $(LDFLAGS)

    $(__mut_builddir)/%.$(oext): $(__mut_builddir)/%.$(cext) $(mull_ir_frontend)
	    $$(call echo-cc,mutate/$$(notdir $$@))
	    $(__set_mullcfg) $(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__mut_builddir)/%.$(oext): $(__node_path)/%.$(cext) $(mull_ir_frontend) | $(__mut_builddir)
	    $$(call echo-cc,mutate/$$(notdir $$@))
	    $(__set_mullcfg) $(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__mut_builddir)/%.$(oext): $(srcdir)/%.$(cext) $(mull_ir_frontend) | $(__mut_builddir)
	    $$(call echo-cc,mutate/$$(notdir  $$@))
	    $(__set_mullcfg) $(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__mut_builddir)/%.$(oext): $(inspectdir)/%.$(cext) $(mull_ir_frontend) | $(__mut_builddir)
	    $$(call echo-cc,mutate/$$(notdir $$@))
	    $(__set_mullcfg) $(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__mut_builddir)/%.$(oext): $(asdir)/%.$(asext) | $(__mut_builddir)
	    $$(call echo-as,mutate/$$(notdir $$@))
	    $(AS) -o $$@ $$< $(ASFLAGS) $(CPPFLAGS)

    mutate: $(__mut)

    $(call pop,LDFLAGS)
    $(call pop,CPPFLAGS)
    $(call pop,CFLAGS))
endef

$(call include-node,algorithm)
$(call include-node,arena)
$(call include-node,bits)
$(call include-node,btmap)
$(call include-node,btree)
$(call include-node,mem)
$(call include-node,hashmap)
$(call include-node,hashtab)
$(call include-node,rbmap)
$(call include-node,rbtree)
$(call include-node,ringdeque)
$(call include-node,stack)
$(call include-node,svec)
$(call include-node,vec)

$(call include-node-if,$(__simd_enable),avx2)

$(call include-node-if-not,$(__simd_enable),simdemu)

$(call pop,LDLIBS)
$(call pop,LDFLAGS)
$(call pop,CPPFLAGS)

$(testsccbuilddir)/%.$(oext): $(__scc_srcdir)/%.$(cext) | $(testsccbuilddir)
	$(call echo-cc,$(notdir $@))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
