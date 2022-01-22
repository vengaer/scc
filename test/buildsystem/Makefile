ifdef __node

__chk_buildsystem := $(__node_builddir)/.check.stamp
PYTESTFLAGS       += --project-root=$(root)

check: $(__chk_buildsystem)

$(__chk_buildsystem): $(call reeval,__node_path,$(__chk_buildsystem))
$(__chk_buildsystem): $(call reeval,PYTESTFLAGS,$(__chk_buildsystem))
$(__chk_buildsystem): | $(__node_builddir)
	$(PYTEST) $(PYTESTFLAGS) $(__node_path)
	$(TOUCH) $@

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
