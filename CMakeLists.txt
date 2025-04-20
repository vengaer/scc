cmake_minimum_required(VERSION 3.16)

include (cmake/version.cmake)

option (SCC_SIMD "SIMD support" ON)

scc_version (SCC_VERSION_MAJOR SCC_VERSION_MINOR SCC_VERSION_PATCH)

project (SCC
    VERSION ${SCC_VERSION_MAJOR}.${SCC_VERSION_MINOR}.${SCC_VERSION_PATCH}
    DESCRIPTION "Generic data collections for C"
    LANGUAGES C ASM
)

if (NOT CMAKE_C_STANDARD)
  set (CMAKE_C_STANDARD 99)
endif()

set (CMAKE_C_STANDARD_REQUIRED ON)
set (CMAKE_C_EXTENSIONS OFF)
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

if (SCC_SIMD)
    add_subdirectory (arch)
endif()

find_package(Python COMPONENTS Interpreter Development)

include (cmake/config.cmake)

file (GLOB SCC_C_SRC lib/*.c)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

add_library (scc SHARED
    ${SCC_ASM_SRC}
    ${SCC_C_SRC}
)
add_dependencies (scc config)

add_library (scc_static STATIC
    ${SCC_ASM_SRC}
    ${SCC_C_SRC}
)
add_dependencies (scc_static config)

if (UNIX)
    set_target_properties (scc_static
        PROPERTIES
        OUTPUT_NAME scc
    )
endif()
