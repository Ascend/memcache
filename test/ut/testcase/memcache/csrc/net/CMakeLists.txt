file(GLOB_RECURSE net_engine_src
        ${CMAKE_CURRENT_SOURCE_DIR}/net_engine.cpp
)
ADD_UNIT_TEST(test_net_engine
        FILES ${net_engine_src} ${TEST_SRC_PATHS}/ut_main.cpp
        INCLUDE_PATHS  ${TEST_INCLUDE_PATHS}
        LINK_LIBRARIES ${TEST_DEPEND_LIBS}
        BUILD_FLAGS -fpermissive --coverage
)
