
tidydirs   := $(srcdir) $(unitdir) $(panicdir) $(headerdir)
tidysrc    := $(foreach __d,$(tidydirs),$(wildcard $(__d)/*.$(cext))\
                                        $(wildcard $(__d)/*.$(hext)))
pylintsrc  := $(wildcard $(pyscripts)/*$(pyext))

tidyenable := clang-analyzer misc performance portability cert readability
tidyignore := clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling \
              readability-function-cognitive-complexity                            \
              readability-uppercase-literal-suffix

__comma    :=,
tidychecks := $(subst $(subst ,, ),$(__comma),$(addsuffix -*,$(tidyenable)) $(addprefix -,$(tidyignore)))

define tidy-rules
$(strip
    $(foreach __s,$(tidysrc),
        $(eval
            __lint_$(__s): $(__s)
	            $$(info [TIDY] $$(notdir $$^))
	            $$(TIDY) $$(TIDYFLAGS) $$^ -- $$(CPPFLAGS)

            lint: __lint_$(__s)))

    $(foreach __s,$(pylintsrc),
        $(eval
            __lint_$(__s): $(__s)
	            $$(info [PYLINT] $$(notdir $$^))
	            $$(PYLINT) $$(PYLINTFLAGS) $$^

            lint: __lint_$(__s))))
endef

$(call tidy-rules)
