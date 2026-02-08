cmake_minimum_required(VERSION 3.16)

include (cmake/version.cmake)
include (CheckFunctionExists)

option (SCC_SIMD "SIMD support" ON)
option (SCC_NO_LIBM "Disallow linking against libm" OFF)

scc_version (SCC_VERSION_MAJOR SCC_VERSION_MINOR SCC_VERSION_PATCH)

project (scc
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

set (SCC_HAVE_LIBM OFF)
if (NOT SCC_NO_LIBM AND NOT NATURAL_LOG_EXISTS AND NOT NEED_LINKING_AGAINST_LIBM)
    CHECK_FUNCTION_EXISTS(log NATURAL_LOG_EXISTS)
    if (NOT NATURAL_LOG_EXISTS)
        unset (NATURAL_LOG_EXISTS CACHE)
        list (APPEND CMAKE_REQUIRED_LIBRARIES m)
        CHECK_FUNCTION_EXISTS(log NATURAL_LOG_EXISTS)
        if (NATURAL_LOG_EXISTS)
            set (NEED_LINKING_AGAINST_LIBM True CACHE BOOL "" FORCE)
            set (SCC_HAVE_LIBM ON)
        else ()
            message (WARNING "Could not find libm")
        endif ()
    endif()
endif()

find_package(Python3 COMPONENTS Interpreter Development)

include (cmake/config.cmake)

file (GLOB SCC_C_SRC lib/*.c)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

file (GLOB SCC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/scc/*.h)
list (REMOVE_DUPLICATES SCC_HEADERS)

add_library (scc SHARED
    ${SCC_ASM_SRC}
    ${SCC_C_SRC}
    ${SCC_HEADERS}
)
add_dependencies (scc config)
add_library (scc_static STATIC
    ${SCC_ASM_SRC}
    ${SCC_C_SRC}
    ${SCC_HEADERS}
)
add_dependencies (scc_static config)
if (UNIX)
    set_target_properties (scc_static
        PROPERTIES
        OUTPUT_NAME scc
    )
endif()

if (NOT SCC_NO_LIBM AND NEED_LINKING_AGAINST_LIBM)
    target_link_libraries(scc m)
    target_link_libraries(scc_static m)
endif()


install (
    TARGETS scc scc_static
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

list (APPEND SCC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/scc/config.h)
list (REMOVE_DUPLICATES SCC_HEADERS)
install (
    FILES ${SCC_HEADERS}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/scc
)
