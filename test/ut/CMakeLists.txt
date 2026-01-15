file(GLOB_RECURSE UT_SOURCE_FILES *.cpp
        testcase/*
        ../stub/*
        ${PROJECT_SMEM_SRC_BASE}/csrc/common/*
        ${PROJECT_SMEM_SRC_BASE}/csrc/config_store/*
        ${PROJECT_SMEM_SRC_BASE}/csrc/net/*
        ${PROJECT_SMEM_SRC_BASE}/csrc/smem_bm/*
        ${PROJECT_SMEM_SRC_BASE}/csrc/smem_shm/*
        ${PROJECT_SMEM_SRC_BASE}/csrc/smem.cpp
        ${PROJECT_MMC_SRC_BASE}/csrc/client/*
        ${PROJECT_MMC_SRC_BASE}/csrc/common/*
        ${PROJECT_MMC_SRC_BASE}/csrc/config/*
        ${PROJECT_MMC_SRC_BASE}/csrc/local_service/*
        ${PROJECT_MMC_SRC_BASE}/csrc/meta_service/*
        ${PROJECT_MMC_SRC_BASE}/csrc/entities/*
        ${PROJECT_MMC_SRC_BASE}/csrc/net/*
        ${PROJECT_MMC_SRC_BASE}/csrc/log/*
        ${PROJECT_MMC_SRC_BASE}/csrc/proto/*
        ${PROJECT_MMC_SRC_BASE}/csrc/under_api/*
        ${PROJECT_MMC_SRC_BASE}/csrc/under_api/mf_smem/*
        ${PROJECT_MMC_SRC_BASE}/csrc/mmc.cpp
        ${PROJECT_MMC_SRC_BASE}/csrc/mmc_client.cpp
        ${PROJECT_MMC_SRC_BASE}/csrc/mmc_service.cpp
)

include_directories(
        ${PROJECT_SMEM_SRC_BASE}/include/host
        ${PROJECT_SMEM_SRC_BASE}/csrc
        ${PROJECT_SMEM_SRC_BASE}/csrc/net
        ${PROJECT_SMEM_SRC_BASE}/csrc/common
        ${PROJECT_SMEM_SRC_BASE}/csrc/config_store
        ${PROJECT_SMEM_SRC_BASE}/csrc/under_api/hybm_core
        ${PROJECT_MMC_SRC_BASE}/include
        ${PROJECT_MMC_SRC_BASE}/include/cpp
        ${PROJECT_MMC_SRC_BASE}/csrc/
        ${PROJECT_MMC_SRC_BASE}/csrc/common
        ${PROJECT_MMC_SRC_BASE}/csrc/client
        ${PROJECT_MMC_SRC_BASE}/csrc/config
        ${PROJECT_MMC_SRC_BASE}/csrc/net
        ${PROJECT_MMC_SRC_BASE}/csrc/log
        ${PROJECT_MMC_SRC_BASE}/csrc/entities
        ${PROJECT_MMC_SRC_BASE}/csrc/net/acc_links_impl
        ${PROJECT_MMC_SRC_BASE}/csrc/meta_service
        ${PROJECT_MMC_SRC_BASE}/csrc/entities
        ${PROJECT_MMC_SRC_BASE}/csrc/local_service
        ${PROJECT_MMC_SRC_BASE}/csrc/proto
        ${PROJECT_MMC_SRC_BASE}/csrc/under_api/mf_smem
        ${PROJECT_MMC_SRC_BASE}/../util/csrc
        ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include/
)

FUNCTION(ADD_UNIT_TEST TEST_NAME)
    set(ONE_VALUE_ARGS PREFIX)
    set(MULTI_VALUE_ARGS FILES BUILD_FLAGS DEPENDS INCLUDE_PATHS LINK_LIBRARIES THIRDPARTY_DEPENDS)
    cmake_parse_arguments(ADD_UNIT_TEST "" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})
    set(TEST_TARGET_NAME ${TEST_NAME})
    add_executable(${TEST_TARGET_NAME} ${ADD_UNIT_TEST_FILES} $<TARGET_OBJECTS:ptracer_object> $<TARGET_OBJECTS:config_store_object>)
    target_link_directories(${TEST_TARGET_NAME} PUBLIC ${PROJECT_OUTPUT_PATH}/acc_links/lib64/)
    target_link_directories(${TEST_TARGET_NAME} PUBLIC ${PROJECT_3RDPARTY_BIN_DIR}/spdlog/lib/)
    target_link_directories(${TEST_TARGET_NAME} PUBLIC ${GTEST_BUILD_LIB_DIR}/lib)
    target_link_directories(${TEST_TARGET_NAME} PUBLIC ${PROJECT_OUTPUT_PATH}/memcache/lib64)
    target_link_directories(${TEST_TARGET_NAME} PUBLIC ${PROJECT_OUTPUT_PATH}/smem/lib64)
    target_link_directories(${TEST_TARGET_NAME} PUBLIC ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64)
    TARGET_MOCKCPP_HEADER(${TEST_TARGET_NAME})
    if (NOT EXISTS ${CHECK_FILE_LINKS})
        add_dependencies(${TEST_TARGET_NAME} googletest)
    endif ()
    install(TARGETS ${TEST_TARGET_NAME}
            DESTINATION ${PROJECT_OUTPUT_PATH}/bin/ut/
            PERMISSIONS ${CACHE_INSTALL_EXE_PERMISSIONS})
    IF (ADD_UNIT_TEST_INCLUDE_PATHS)
        target_include_directories(${TEST_TARGET_NAME} PUBLIC ${ADD_UNIT_TEST_INCLUDE_PATHS})
    ENDIF ()

    IF (ADD_UNIT_TEST_LINK_LIBRARIES)
        target_link_libraries(${TEST_TARGET_NAME} ${ADD_UNIT_TEST_LINK_LIBRARIES})
    ENDIF ()

    IF (ADD_UNIT_TEST_DEPENDS)
        add_dependencies(${TEST_TARGET_NAME} ${ADD_UNIT_TEST_DEPENDS})
    ENDIF ()

    IF (ADD_UNIT_TEST_BUILD_FLAGS)
        target_compile_options(${TEST_TARGET_NAME} PUBLIC ${ADD_UNIT_TEST_BUILD_FLAGS})
    ENDIF ()

    add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_TARGET_NAME}>)

ENDFUNCTION()

set(TEST_INCLUDE_PATHS ../../src/memcache/)
set(TEST_SRC_PATHS ${CMAKE_CURRENT_SOURCE_DIR})
set(TEST_DEPEND_LIBS gtest dl acc_tcp_net_static mmc_static gcov pthread)

add_subdirectory(testcase/memcache/csrc/net)
add_subdirectory(testcase/memcache/csrc/meta_service)
add_subdirectory(testcase/memcache/csrc/local_service)
add_subdirectory(testcase/memcache/csrc/common)
add_subdirectory(testcase/memcache/csrc/config)
add_subdirectory(testcase/memcache/csrc/entities)
add_subdirectory(testcase/memcache/csrc/log)
add_subdirectory(testcase/memcache/csrc/)