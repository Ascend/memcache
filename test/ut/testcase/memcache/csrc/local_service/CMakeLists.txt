file(GLOB_RECURSE local_service_src
        ${CMAKE_CURRENT_SOURCE_DIR}/test_local_handler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_mmcache_store.cpp
)
ADD_UNIT_TEST(test_local_service
        FILES ${local_service_src} ${TEST_SRC_PATHS}/ut_main.cpp
        INCLUDE_PATHS ${TEST_INCLUDE_PATHS}
        LINK_LIBRARIES ${TEST_DEPEND_LIBS}
        BUILD_FLAGS -fpermissive --coverage
)
add_dependencies(test_local_service mock_mf_smem)
set_tests_properties(test_local_service PROPERTIES
        ENVIRONMENT "MEMFABRIC_HYBRID_EXTEND_LIB_PATH=${PROJECT_OUTPUT_PATH}/test/mock_mf_smem/lib64"
)
