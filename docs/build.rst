Building from Source
====================

Building ``scc`` requires ``conftool``. While this is included as a submodule and easily installed via
`cargo <https://man.archlinux.org/man/cargo.1.en>`_, it is far less tedious to use the accompanying Docker
image. To build the image, enter an interactive container and build ``scc``, run the following

.. code:: shell

    make docker
    make -j$(nproc)

The libraries are found in the root of the ``build`` directory.
