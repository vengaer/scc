ifdef __node

__chk_buildsystem := $(__node_builddir)/.check.stamp
buildsystestdir   := $(__node_path)

$(__chk_buildsystem): $(__all_mkfiles) $(wildcard $(__node_path)/*.$(pyext)) | $(__node_builddir)
	$(PYTEST) $(PYTESTFLAGS) --project-root=$(root) $(buildsystestdir)
	$(TOUCH) $@

check: $(__chk_buildsystem)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
