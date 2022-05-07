ifndef _Barrier_mk
_Barrier_mk := _

# Release include barrier used to block its corresponding
# acquire barrier until $(1) has been included
define __include-barrier-rel
$(strip
    $(if $(wildcard $(1)),
        $(eval $(1).barrier.rel := _))
    $(eval -include $(1)))
endef

# Synchronize with corresponding release barrier, expanding to $(2)
# only if $(1) has been included
__include-barrier-acq = $(if $($(1).barrier.rel),$(2))

endif # _Barrier_mk
