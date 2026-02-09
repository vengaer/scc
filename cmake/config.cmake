set (SCCONFIG ${CMAKE_CURRENT_SOURCE_DIR}/scripts/py/scconfig.py)
set (CONFOPTS -g SCC_CONFIG_H -c ${CMAKE_CURRENT_BINARY_DIR}/.scconfig.cmake.cache -l ${CMAKE_CURRENT_BINARY_DIR}/.scconfig.cmake.lock)

set (CONFHDR ${CMAKE_CURRENT_SOURCE_DIR}/scc/config.h)

add_custom_target (.config.init.cmake.stamp
    COMMAND ${Python3_EXECUTABLE} ${SCCONFIG} ${CONFOPTS} -o ${CONFHDR} init
)

add_custom_target (.config.version.cmake.stamp
    COMMAND ${Python3_EXECUTABLE} ${SCCONFIG} ${CONFOPTS} add SCC_VERSION_MAJOR ${SCC_VERSION_MAJOR} -C "Library major version"
    COMMAND ${Python3_EXECUTABLE} ${SCCONFIG} ${CONFOPTS} add SCC_VERSION_MINOR ${SCC_VERSION_MINOR} -C "Library minor version"
    COMMAND ${Python3_EXECUTABLE} ${SCCONFIG} ${CONFOPTS} add SCC_VERSION_PATCH ${SCC_VERSION_PATCH} -C "Library patch version"
    DEPENDS .config.init.cmake.stamp
)

add_custom_target(.config.libm.cmake.stamp
    COMMAND ${Python3_EXECUTABLE} ${SCCONFIG} ${CONFOPTS} add SCC_HAVE_LIBM -C "Linked against libm"
    DEPENDS .config.version.cmake.stamp
)

add_custom_target(.config.bitarch.cmake.stamp
    COMMAND ${Python3_EXECUTABLE} ${SCCONFIG} ${CONFOPTS} add SCC_BITARCH_$<IF:$<STREQUAL:${CMAKE_SIZEOF_VOID_P},8>,64,32> -C "System architecture"
    DEPENDS .config.$<IF:$<BOOL:${SCC_HAVE_LIBM}>,libm,version>.cmake.stamp
)

add_custom_target(.config.u32.cmake.stamp
    COMMAND ${Python3_EXECUTABLE} ${SCCONFIG} ${CONFOPTS} add SCC_$<IF:$<BOOL:${HAVE_UINT32_T}>,HAVE,NO>_UINT32_T -C "Have uint32_t?"
    DEPENDS .config.bitarch.cmake.stamp
)

add_custom_target(.config.u64.cmake.stamp
    COMMAND ${Python3_EXECUTABLE} ${SCCONFIG} ${CONFOPTS} add SCC_$<IF:$<BOOL:${HAVE_UINT64_T}>,HAVE,NO>_UINT64_T -C "Have uint64_t?"
    DEPENDS .config.u32.cmake.stamp
)

add_custom_target (.config.simd.cmake.stamp
    COMMAND ${Python3_EXECUTABLE} ${SCCONFIG} ${CONFOPTS} add SCC_SIMD_ISA ${SCC_SIMD_ISA} -C "SIMD instruction set architecture"
    COMMAND ${Python3_EXECUTABLE} ${SCCONFIG} ${CONFOPTS} add SCC_HWVEC_SIZE ${SCC_VECSIZE} -C "Size of hardware SIMD vectors"
    DEPENDS .config.u64.cmake.stamp
)

add_custom_target (config
    COMMAND ${Python3_EXECUTABLE} ${SCCONFIG} ${CONFOPTS} commit
    DEPENDS .config.$<IF:$<BOOL:${SCC_SIMD_SUPPORTED}>,simd,u64>.cmake.stamp
)
