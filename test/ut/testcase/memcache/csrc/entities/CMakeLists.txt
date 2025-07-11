file(GLOB_RECURSE entities_src
        ${CMAKE_CURRENT_SOURCE_DIR}/test_lookup_map.cpp
)
ADD_UNIT_TEST(test_entities
        FILES ${entities_src} ${TEST_SRC_PATHS}/ut_main.cpp
        INCLUDE_PATHS  ${TEST_INCLUDE_PATHS}
        LINK_LIBRARIES ${TEST_DEPEND_LIBS}
        BUILD_FLAGS -fpermissive --coverage
)
