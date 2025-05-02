.. _cmake_build:

Build Using cmake
=================

.. code:: shell

    make docker  # Optional, build and run arch docker image
    mkdir build
    cmake -E chdir build cmake ..
    cmake --build build --config Release

and to install it

.. code:: shell

    sudo make -C build install
