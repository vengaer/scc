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
