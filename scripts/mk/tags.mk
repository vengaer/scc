CTAGS      := ctags
tagsexcl   := .git build docs submodules/benchmark/build
CTAGSFLAGS := -R $(addprefix --exclude=,$(tagsexcl))

tags: $(submodules)
