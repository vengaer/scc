ifndef __Path_mk
__Path_mk := _

include $(mkscripts)/expr.mk
include $(mkscripts)/vars.mk

__path_sep := /

# Convert absolute path $(1) to a one relative $(2)
# $(call path-relative-to,START,ABSPATH)
define path-relative-to
$(strip
    $(eval __common_prefix :=)
    $(foreach __w,$(subst $(__path_sep), ,$(1)),
        $(if $(findstring |_|$(__common_prefix)$(__path_sep)$(__w),|_|$(2)),
            $(eval __common_prefix := $(__common_prefix)$(__path_sep)$(__w)),
          $(eval __common_prefix := $(__common_prefix)|<|)))
    $(eval __common_prefix := $(firstword $(subst |<|, ,$(__common_prefix))))
    $(eval __nup := $(words $(subst $(__path_sep), ,$(subst $(__common_prefix),,$(1)))))
    $(1)$(__path_sep)$(subst $(space),$(__path_sep),$(call for-in-range,$(__nup),..))$(subst $(__common_prefix),,$(2)))
endef

endif # __Path_mk

