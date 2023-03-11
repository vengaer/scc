ifndef __Lint_mk
__Lint_mk := -

include $(mkscripts)/ext.mk
include $(mkscripts)/vars.mk

__lint_py    := $(patsubst $(pyscripts)/%,$(builddir)/.lint.%.stamp,$(wildcard $(pyscripts)/*.$(pyext)))

tidyenable   := clang-analyzer misc performance portability cert readability
tidyignore   := clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling \
                readability-function-cognitive-complexity                            \
                readability-uppercase-literal-suffix readability-magic-numbers       \
                cert-dcl16-c readability-identifier-length

tidychecks   := $(subst $(subst ,, ),$(comma),$(addsuffix -*,$(tidyenable)) $(addprefix -,$(tidyignore)))

__mcompat    := $(pyscripts)/mcompatchk.$(pyext)
__mcompatcfg := $(root)/.mcompat.$(jsonext)

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

            $(__lint_$(__s)): $(__s) $(config_header) $(__all_mkfiles) | $(__node_builddir) $(submodules)
	            $$(call echo-lint,$$(notdir $$<))
	            $(TIDY) $(TIDYFLAGS) $$< -- $(CPPFLAGS) $(CFLAGS)
	            $(TOUCH) $$@

            lint: $(__lint_$(__s))))
    $(foreach __s,$(wildcard $(__node_path)/*.$(hext)),
        $(eval __compatchk_$(__s) := $(__node_builddir)/.mcompatchk.$(notdir $(__s)).stamp)

        $(__compatchk_$(__s)): $(__s) $(__mcompat) $(__mcompatcfg) $(config_header) $(__all_mkfiles) | $(__node_builddir) $(submodules)
	        $$(call echo-mcompat,$$(notdir $$<))
	        $(PYTHON) $(__mcompat) -c $(__mcompatcfg) -I$(root)/submodules/pycparser/utils/fake_libc_include $$<
	        $(TOUCH) $$@

        lint: $(__compatchk_$(__s))))
endef

endif
