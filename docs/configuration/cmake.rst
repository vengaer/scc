.. _cmake_config:

cmake Configuration Options
===========================

The following cmake options may be used to control scc.

.. list-table:: Comparison return values
    :header-rows: 1

    * - Option
      - Meaning
      - Default
    * - SCC_SIMD
      - Use hardware SIMD [1]_.
      - ON
    * - `CMAKE_C_STANDARD`_
      - The C standard to use, must be at least 99
      - 99
    * - `CMAKE_INSTALL_PREFIX`_
      - Determines where the library is installed
      - System dependent, see the cmake documentation

.. [1] If supported. This currently comprises only 64-bit Linux systems running on x86-64 processors.

.. _CMAKE_C_STANDARD: https://cmake.org/cmake/help/latest/variable/CMAKE_C_STANDARD.html
.. _CMAKE_INSTALL_PREFIX: https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html
