ifndef __Echo_mk
__Echo_mk := _

__space   := $(subst ,, )

echo-raw   = $(info $(__space)$(1) $(2))
echo-gen   = $(call echo-raw,GEN, $(1))
echo-as    = $(call echo-raw,AS,  $(1))
echo-cc    = $(call echo-raw,CC , $(1))
echo-cxx   = $(call echo-raw,CXX, $(1))
echo-ld    = $(call echo-raw,LD,  $(1))
echo-lint  = $(call echo-raw,LINT,$(1))
echo-ln    = $(call echo-raw,LN,  $(1))
echo-ar    = $(call echo-raw,AR,  $(1))
echo-exec  = $(call-echo-raw,EXEC,$(1))

endif
