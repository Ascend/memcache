add_subdirectory(log)

####################################################################
# MemFabric_Hybrid
####################################################################

set(_ACTUAL_BUILD_TESTS ${BUILD_TESTS})
set(BUILD_TESTS OFF)
add_subdirectory(memfabric_hybrid)
set(BUILD_TESTS ${_ACTUAL_BUILD_TESTS})