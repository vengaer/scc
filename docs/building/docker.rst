Docker
======

scc provides a few different docker images which may be used to build the library. All of them
support both the :ref:`cmake <cmake_build>` and :ref:`make <make_build>` build systems.

scc provides a number of different images that may be used when building. These are based on

- Arch Linux
- Debian
- Ubuntu 24.04

These can be built using

.. code-block:: shell

    make docker-image DISTRO=arch         # Arch Linux
    make docker-image DISTRO=debian       # Debian
    make docker-image DISTRO=ubuntu24.04  # Ubuntu

If you wish to fire up an interactive container, you may replace
``docker-image`` with ``docker`.
