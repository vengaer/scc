ifdef __node

unitydir       := $(__node_path)/unity
unityinc       := $(unitydir)/src
unityalib      := $(unitydir)/libunity.a
unitygenerator := $(unitydir)/auto/generate_test_runner.rb

submodules := $(__node_path)/.init.stamp

$(submodules):
	git submodule update --init
	$(TOUCH) $@

$(unityalib): $(call reeval,cpu_cores,$(unityalib))
$(unityalib): $(submodules)
	$(CMAKE) -B $(dir $@) $(dir $@)
	$(MAKE) -C $(dir $@) -j$(cpu_cores)
	$(TOUCH) $@

$(unitygenerator): $(submodules)

.PHONY: __clean_unity
__clean_unity:
	$(MAKE)C $(unitydir) clean

.PHONY: distclean
distclean: $(if $(wildcard $(submodules)),__clean_unity)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
