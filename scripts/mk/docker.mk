ifndef __Docker_mk
__Docker_mk := _

include $(mkscripts)/echo.mk

IMAGE         := scc/build
__image_stamp := $(builddir)/.docker.image.stamp

$(__image_stamp): $(builddir)
	$(call echo-gen,$(IMAGE))
	docker build . -t$(IMAGE) --network=host
	$(TOUCH) $@

docker-image: $(__image_stamp)

docker: $(__image_stamp)
	$(call echo-exec,$(IMAGE))
	docker run --rm -ti --net=host -u$$(id -u) -w=$(root) -v$(root):$(root) $(IMAGE)

endif
