Welcome to scc's documentation!
===============================

.. default-domain:: C



.. code-block:: c

    #include <scc/btmap.h>

    int cmp(void const *l, void const *r) {
        return *(int const *)l - *(int const *)r;
    }

    int main(void) {
        scc_btmap(int, char const *) btmap = scc_btmap_new(int, char const *, cmp);

        assert(scc_btmap_insert(&btmap, 32, "scc"));
        assert(scc_btmap_insert(&btmap, 24, "aims"));
        assert(scc_btmap_insert(&btmap, 78, "to"));
        assert(scc_btmap_insert(&btmap, 78, "to"));

    }

.. toctree::
    :includehidden:
    :maxdepth: 1

    reference/index
