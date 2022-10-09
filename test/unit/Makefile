ifdef __node

runnersuffix    := _runner

$(call push,CFLAGS)
$(call push,CPPFLAGS)
$(call push,LDFLAGS)
$(call push,LDLIBS)

CFLAGS          += -fsanitize=address,undefined
CPPFLAGS        := -I$(unityinc) -I$(testdir) $(filter-out -DNDEBUG,$(CPPFLAGS)) -I$(srcdir)
LDFLAGS         += -fsanitize=address,undefined -L$(unitydir)
LDLIBS          += -lunity -lm

define __unit-obj
$(strip
    $(patsubst $(srcdir)/%.$(cext),$(builddir)/scc/%.$(oext),
        $(wildcard $(patsubst %,$(srcdir)/*_%*.$(cext),$(__node) $($(__node)_deps))))
    $(patsubst $(inspectdir)/%.$(cext),$(inspectbuilddir)/%.$(oext),
        $(wildcard $(inspectdir)/scc_$(__node)*.$(cext)))
    $(patsubst $(asdir)/%.$(asext),$(asbuilddir)/%.$(oext),
        $(wildcard $(patsubst %,$(asdir)/*%*.$(asext),$(__node) $($(__node)_deps)))))
endef

define decl-unit
$(eval
    $(eval __unitsrc   := $(wildcard $(__node_path)/*.$(cext)))
    $(eval __chk       := $(patsubst $(__node_path)/%.$(cext),$(__node_builddir)/.chk.%.stamp,$(__unitsrc)))
    $(eval __runners   := $(patsubst $(__node_path)/%.$(cext),$(__node_builddir)/%$(runnersuffix),$(__unitsrc)))

    # Prevent make from removing generated runners
    .SECONDARY: $(addsuffix .$(cext),$(__runners))

    # Recipies for running tests
    $(foreach __s,$(__chk),
        $(__s): $(call reeval,CPPFLAGS,$(__s))
        $(__s): $(patsubst $(__node_builddir)/.chk.%.stamp,$(__node_builddir)/%$(runnersuffix),$(__s))
	        $$<
	        $(TOUCH) $$@)

    # Runner source generation
    $(__node_builddir)/%$(runnersuffix).$(cext): $(__node_path)/%.$(cext) $(submodules) | $(__node_builddir)
	    $$(call echo-gen,$$(notdir $$@))
	    $(RB) $(unitygenerator) $$< $$@

    $(foreach __r,$(__runners),
        $(__r): $(__r).$(oext) $(patsubst %$(runnersuffix),%.$(oext),$(__r)) $(call __unit-obj) $(unityalib)
	        $$(call echo-ld,$$(notdir $$@))
	        $(LD) -o $$@ $$^ $(LDFLAGS) $(LDLIBS))

    $(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	    $$(call echo-cc,$$(notdir $$@))
	    $(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__node_builddir)/%.$(oext): $(srcdir)/%.$(cext) | $(__node_builddir)
	    $$(call echo-cc,$(__node)/$$(notdir $$@))
	    $(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    check: $(__chk)
)
endef


$(call include-node,algorithm)
$(call include-node,arena)
$(call include-node,bits)
$(call include-node,btree)
$(call include-node,mem)
$(call include-node,hashmap)
$(call include-node,hashtab)
$(call include-node,rbtree)
$(call include-node,ringdeque)
$(call include-node,stack)
$(call include-node,svec)
$(call include-node,vec)

$(call pop,LDLIBS)
$(call pop,LDFLAGS)
$(call pop,CPPFLAGS)
$(call pop,CFLAGS)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
