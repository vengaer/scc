ifndef __Expr_mk
__Expr_mk := _

# Negation, for use with $(if ...)
# $(call not,VARIABLE)
not     = $(if $(1),,_)

# Force reevaluation of $(1) for optional target $(2)
# $(call reeval,VARIABLE[,TARGET])
reeval  = $(eval $(1) := $(eval $(2)$(if $(2),:) $(1) := $($(1)))$($(1)))

# Expand to non-empty if $(1) is empty
# $(call empty,VARIABLE)
empty   = $(if $(call not,$(1)),_,)

# Require that $(2) is set for target $(1)
# $(call require,TARGET,VARIABLE)
require = $(if $(findstring _-$(1)_-,_-$(MAKECMDGOALS)_-),$(if $(call not,$($(2))),$(error $(2) is empty)))

# Assert that $(1) is true (i.e. non-empty). $(2) contains an optional
# error message
# $(call assert,CONDITION[, DESCRIPTION])
assert  = $(if $(call not,$(1)),$(error assertion failed$(if $(2),: $(strip $(2)))))

endif # __Expr_mk
