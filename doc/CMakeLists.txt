# Copyright (c) 2026 Vilhelm Engström
#
# SPDX-License-Identifier: Apache-2.0

find_package(Sphinx 8.2.3 REQUIRED)

set(html_out ${CMAKE_CURRENT_BINARY_DIR}/html)

# -----------
# HTML target

list(APPEND DOC_ENV "SCC_VERSION_MAJOR=${SCC_VERSION_MAJOR}")
list(APPEND DOC_ENV "SCC_VERSION_MINOR=${SCC_VERSION_MINOR}")
list(APPEND DOC_ENV "SCC_VERSION_PATCH=${SCC_VERSION_PATCH}")

sphinx_add_docs(html ALL
    ENVIRONMENT "${DOC_ENV}"
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html"
)
