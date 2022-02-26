ifndef _Refl_mk
_Refl_mk := _

__is_cleaning      := $(findstring clean,$(MAKECMDGOALS))
__is_building_tags := $(findstring tags,$(MAKECMDGOALS))
__skip_arch        := $(or $(__is_cleaning),$(__is_building_tags))
endif
