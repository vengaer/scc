
.PHONY: all
all:

.PHONY: __chk_buildsystem
__chk_buildsystem:
	$(PYTEST) $(PYTESTFLAGS) --root=$(root) $(buildtestdir)

check: __chk_buildsystem
