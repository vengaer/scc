ifndef __Docker_mk
__Docker_mk := _

include $(mkscripts)/echo.mk

DISTRO        ?= arch
IMAGE         := scc/$(DISTRO)-build
__dockerfile  := $(root)/docker/$(DISTRO)/Dockerfile
__image_stamp := $(builddir)/.docker.image.stamp

$(__image_stamp): $(__dockerfile) $(submodules) $(__all_mkfiles) | $(builddir)
	$(call echo-gen,$(IMAGE))
	docker build $(root) -t$(IMAGE) --network=host -f $(__dockerfile)
	$(TOUCH) $@

docker-image: $(__image_stamp)

docker: $(__image_stamp)
	$(call echo-exec,$(IMAGE))
	docker run --rm -ti --net=host -v$(root):$(root) $(IMAGE)

endif
