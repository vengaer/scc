set (sysv_systems Linux Darwin)

if (${CMAKE_SYSTEM_NAME} IN_LIST sysv_systems)
  add_subdirectory (sysv)
endif()
