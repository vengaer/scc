ifndef __Lint_mk
__Lint_mk := -

__lint_py  := $(patsubst $(pyscripts)/%,$(builddir)/.lint.%.stamp,$(wildcard $(pyscripts)/*.$(pyext)))

tidyenable := clang-analyzer misc performance portability cert readability
tidyignore := clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling \
              readability-function-cognitive-complexity                            \
              readability-uppercase-literal-suffix readability-magic-numbers       \
              cert-dcl16-c readability-identifier-length

__comma    :=,
tidychecks := $(subst $(subst ,, ),$(__comma),$(addsuffix -*,$(tidyenable)) $(addprefix -,$(tidyignore)))

$(__lint_py): $$(patsubst $(builddir)/.lint.%.stamp,$(pyscripts)/%,$$@) $(__all_mkfiles) | $(builddir)
	$(call echo-lint,$(notdir $<))
	$(PYLINT) $(PYLINTFLAGS) $<
	$(TOUCH) $@

lint: $(__lint_py)

define decl-lint
$(eval
    $(foreach __ext,$(hext) $(cext),
        $(foreach __s,$(wildcard $(__node_path)/*.$(__ext)),
            $(eval __lint_$(__s) := $(__node_builddir)/.lint.$(notdir $(__s)).stamp)

            $(__lint_$(__s)): $(__s) $(__config_header) $(__all_mkfiles) | $(__node_builddir) $(submodules)
	            $$(call echo-lint,$$(notdir $$<))
	            $(TIDY) $(TIDYFLAGS) $$< -- $(CPPFLAGS) $(CFLAGS)
	            $(TOUCH) $$@

            lint: $(__lint_$(__s)))))
endef

endif
