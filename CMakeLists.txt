cmake_minimum_required(VERSION 3.16)

include (cmake/version.cmake)

scc_version(SCC_VERSION_MAJOR SCC_VERSION_MINOR SCC_VERSION_PATCH)

project(SCC
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

add_subdirectory (arch)
