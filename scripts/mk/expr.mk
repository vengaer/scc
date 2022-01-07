# Negation, for use with $(if ...)
# $(call not,VARIABLE)
not    = $(if $(1),,_)

# Force reevaluation of $(1) for optional target $(2)
# $(call reeval,VARIABLE[,TARGET])
reeval = $(eval $(1) := $(eval $(2)$(if $(2),:) $(1) := $($(1)))$($(1)))

# Expand to non-empty if $(1) is empty
# $(call empty,VARIABLE)
empty  = $(if $(call not,$(1)),_,)
