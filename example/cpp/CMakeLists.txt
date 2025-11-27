cmake_minimum_required(VERSION 3.12.0)
project(memcache_cpp_test VERSION 1.0 LANGUAGES CXX C)

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

# 递归查找所有头文件目录
file(GLOB_RECURSE ALL_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${MF_PATH}/*.h"
)

# 提取头文件目录并去重，同时过滤掉路径包含XXX的目录
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
)

# 创建可执行文件
add_executable(memcache_cpp_test memcache_cpp_test.cpp)
foreach (SO ${ALL_SO})
    get_filename_component(FILENAME ${SO} NAME)
    string(FIND "${FILENAME}" "memcache" FILENAME_MATCH)
    if (NOT FILENAME_MATCH EQUAL -1)
        MESSAGE(STATUS "${SO}")
        target_link_libraries(memcache_cpp_test PUBLIC ${SO})
    endif ()
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