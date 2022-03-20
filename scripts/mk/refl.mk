ifndef _Refl_mk
_Refl_mk := _

__is_cleaning      := $(findstring clean,$(MAKECMDGOALS))
__is_building_tags := $(findstring tags,$(MAKECMDGOALS))
__is_building_docs := $(findstring docs,$(MAKECMDGOALS))
__is_docker_target := $(findstring docker,$(MAKECMDGOALS))
__skip_arch        := $(__is_cleaning)$(__is_building_tags)$(__is_building_docs)$(__is_docker_target)
endif
