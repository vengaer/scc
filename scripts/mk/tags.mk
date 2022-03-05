ifndef __Tags_mk
__Tags_mk := _

CTAGS      := ctags
tagsexcl   := .git build docs submodules/benchmark/build
CTAGSFLAGS := -R $(addprefix --exclude=,$(tagsexcl))

tags: $(submodules)

endif # __Tags_mk
