rbtree_deps  := arena rbtree_inspect svec
hashtab_deps  = $(patsubst $(assrcdir)/scc_%.$(asext),%,$(wildcard $(assrcdir)/*.$(asext)))
