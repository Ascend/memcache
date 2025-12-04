if (("${BUILD_CPP_EXAMPLE}" STREQUAL "") OR ("${BUILD_CPP_EXAMPLE}" STREQUAL "OFF"))
    message(STATUS "BUILD_CPP_EXAMPLE is OFF.")
    cmake_minimum_required(VERSION 3.12.0)
    project(memcache_cpp_test VERSION 1.0 LANGUAGES CXX C)
    set(CMAKE_SKIP_RPATH TRUE)
    set(CMAKE_CXX_STANDARD 17)
    set(MF_PATH $ENV{MEMFABRIC_HYBRID_HOME_PATH})
    message(STATUS "MF_PATH: ${MF_PATH}")
    if ("${MF_PATH}" STREQUAL "")
        message(FATAL_ERROR "MEMFABRIC_HYBRID_HOME_PATH memfabric_hybrid path not found!")
    endif ()
    set(ASCEND_CANN_PACKAGE_PATH $ENV{ASCEND_HOME_PATH})
    if ("${ASCEND_CANN_PACKAGE_PATH}" STREQUAL "")
        message(FATAL_ERROR "ASCEND_HOME_PATH ascend cann path not found!")
    endif ()

    set(MCC_PATH $ENV{MEMCACHE_HYBRID_HOME_PATH})
    message(STATUS "MCC_PATH: ${MCC_PATH}")
    if ("${MCC_PATH}" STREQUAL "")
        message(FATAL_ERROR "MEMCACHE_HYBRID_HOME_PATH memcache path not found!")
    endif ()

    # 递归查找所有头文件目录
    file(GLOB_RECURSE ALL_HEADERS
            "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
            "${MF_PATH}/*.h"
            "${MCC_PATH}/*.h"
    )

    set(INCLUDE_DIRS "")
    foreach (header ${ALL_HEADERS})
        # 获取头文件所在目录
        get_filename_component(HEADER_DIR ${header} PATH)
        list(APPEND INCLUDE_DIRS ${HEADER_DIR})
    endforeach ()

    # 去重包含目录
    list(REMOVE_DUPLICATES INCLUDE_DIRS)

    # 添加包含目录
    include_directories(${INCLUDE_DIRS})

    list(LENGTH INCLUDE_DIRS INCLUDE_DIRS_COUNT)
    message(STATUS "Project configured with ${INCLUDE_DIRS_COUNT} include directories")

    # 递归查找所有头文件目录
    file(GLOB_RECURSE ALL_SO
            "${MF_PATH}/*.so"
            "${MCC_PATH}/*.so"
    )

    # 创建可执行文件
    add_executable(memcache_cpp_test memcache_cpp_test.cpp)
    foreach (SO ${ALL_SO})
        MESSAGE(STATUS "${SO}")
        get_filename_component(FILENAME ${SO} NAME)
        get_filename_component(FILEDIR ${SO} DIRECTORY)

        string(FIND "${FILENAME}" "python" FILENAME_MATCH)
        if (NOT FILENAME_MATCH EQUAL -1)
            continue()
        endif ()

        target_link_directories(memcache_cpp_test PUBLIC ${FILEDIR})
        target_link_libraries(memcache_cpp_test PUBLIC ${FILENAME})
    endforeach ()
    # 查找 Python 开发库
    find_package(Python3 REQUIRED COMPONENTS Development)
    target_link_libraries(memcache_cpp_test PUBLIC
            Python3::Python  # 添加 Python 库
            pthread
            dl
    )
    message(STATUS "${INCLUDE_DIRS}")
    target_include_directories(memcache_cpp_test PRIVATE ${INCLUDE_DIRS})
    target_compile_options(memcache_cpp_test PRIVATE -W)
else ()
    message(STATUS "BUILD_CPP_EXAMPLE is ${BUILD_CPP_EXAMPLE}.")
    set(CMAKE_SKIP_RPATH FALSE)
    # 递归查找所有头文件目录
    file(GLOB_RECURSE ALL_HEADERS
            "${PROJECT_SOURCE_DIR}/src/memcache/include/*.h"
    )

    set(INCLUDE_DIRS "")
    foreach (header ${ALL_HEADERS})
        # 获取头文件所在目录
        get_filename_component(HEADER_DIR ${header} PATH)
        list(APPEND INCLUDE_DIRS ${HEADER_DIR})
    endforeach ()

    # 去重包含目录
    list(REMOVE_DUPLICATES INCLUDE_DIRS)

    # 添加包含目录
    include_directories(${INCLUDE_DIRS})

    list(LENGTH INCLUDE_DIRS INCLUDE_DIRS_COUNT)
    message(STATUS "Project configured with ${INCLUDE_DIRS_COUNT} include directories")


    # 创建可执行文件
    add_executable(memcache_cpp_test memcache_cpp_test.cpp)
    target_link_libraries(memcache_cpp_test PUBLIC mmc_shared)
    # 查找 Python 开发库
    find_package(Python3 REQUIRED COMPONENTS Development)
    target_link_libraries(memcache_cpp_test PUBLIC
            Python3::Python  # 添加 Python 库
            pthread
            dl
    )
    message(STATUS "${INCLUDE_DIRS}")
    target_include_directories(memcache_cpp_test PRIVATE ${INCLUDE_DIRS})
    target_compile_options(memcache_cpp_test PRIVATE -W)
    message(STATUS "BUILD_CPP_EXAMPLE is ON install: ${TARGET_INSTALL_DIR}/memcache/bin")
    install(TARGETS memcache_cpp_test
            RUNTIME DESTINATION ${TARGET_INSTALL_DIR}/memcache/bin
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
endif ()