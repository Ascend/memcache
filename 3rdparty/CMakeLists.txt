####################################################################
# MemFabric_Hybrid
####################################################################

set(_ACTUAL_BUILD_TESTS ${BUILD_TESTS})
set(BUILD_TESTS OFF)
set(BUILD_FROM_MEMCACHE ON)
add_subdirectory(memfabric_hybrid)
set(BUILD_TESTS ${_ACTUAL_BUILD_TESTS})