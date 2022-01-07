submodules := $(submoddir)/.init.stamp

.PHONY: all
all:

$(submodules):
	git submodule update --init
	$(TOUCH) $@
